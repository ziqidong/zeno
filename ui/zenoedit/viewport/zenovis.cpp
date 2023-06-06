#include "camerakeyframe.h"
#include "viewportwidget.h"
#include "cameracontrol.h"
#include "../zenomainwindow.h"
#include "../launch/corelaunch.h"
#include "../timeline/ztimeline.h"
#include <zeno/extra/GlobalState.h>
#include <zeno/extra/GlobalComm.h>
#include <zeno/utils/logger.h>
#include <zeno/zeno.h>
#include <zenovis/ObjectsManager.h>
#include <zenovis/RenderEngine.h>

Zenovis::Zenovis(QObject *parent)
    : QObject(parent)
    , m_solver_frameid(0)
    , m_solver_interval(0)
    , m_render_fps(0)
    , m_resolution(QPoint(1,1))
    , m_cache_frames(10)
    , m_playing(false)
    , m_camera_keyframe(nullptr)
    , m_currentCameraName("none")
{
}

void Zenovis::loadGLAPI(void *procaddr)
{
    zenovis::Session::load_opengl_api(procaddr);
}

void Zenovis::initializeGL()
{
    session = std::make_unique<zenovis::Session>();
}

void Zenovis::paintGL()
{
    int frameid = session->get_curr_frameid();
    doFrameUpdate();
    session->new_frame();
    emit frameDrawn(frameid);
}

//void Zenovis::recordGL(const std::string& record_path, int nsamples)
//{
    //session->new_frame_offline(record_path, nsamples);
//}

int Zenovis::getCurrentFrameId()
{
    return session->get_curr_frameid();
}

void Zenovis::updatePerspective(QVector2D const &resolution, PerspectiveInfo const &perspective)
{
    m_resolution = resolution;
    m_perspective = perspective;
    if (session) {
        session->set_window_size(m_resolution.x(), m_resolution.y());
        session->look_perspective(m_perspective.cx, m_perspective.cy, m_perspective.cz,
                                  m_perspective.theta, m_perspective.phi, m_perspective.radius,
                                  m_perspective.fov, m_perspective.ortho_mode,
                                  m_perspective.aperture, m_perspective.focalPlaneDistance);
    }
    emit perspectiveUpdated(perspective);
}

void Zenovis::updateCameraFront(QVector3D center, QVector3D front, QVector3D up) {
    if (session) {
        session->look_to_dir(center.x(), center.y(), center.z(),
                             front.x(), front.y(), front.z(),
                             up.x(), up.y(), up.z());
    }
}

void Zenovis::setCurrentCamera(QString camName) {
    m_currentCameraName = camName;
}

void Zenovis::startPlay(bool bPlaying)
{
    m_playing = bPlaying;
}

bool Zenovis::isPlaying() const
{
    return m_playing;
}

zenovis::Session *Zenovis::getSession() const
{
    return session.get();
}

int Zenovis::setCurrentFrameId(int frameid)
{
    if (frameid < 0 || !session)
        frameid = 0;

    auto &globalComm = zeno::getSession().globalComm;
    std::pair<int, int> frameRg = globalComm->frameRange();
    int numOfFrames = globalComm->numOfFinishedFrame();
    if (numOfFrames > 0)
    {
        int endFrame = frameRg.first + numOfFrames - 1;
        if (frameid < frameRg.first) {
            frameid = frameRg.first;
        } else if (frameid > endFrame) {
            frameid = endFrame;
        }
    } else {
        frameid = 0;
    }

    zeno::log_trace("now frame {}/{}", frameid, frameRg.second);
    int old_frameid = session->get_curr_frameid();
    session->set_curr_frameid(frameid);
    if (old_frameid != frameid) {
        if (m_camera_keyframe && m_camera_control) {
            PerspectiveInfo r;
            if (m_camera_keyframe->queryFrame(frameid, r)) {
                m_camera_control->setKeyFrame();
                m_camera_control->updatePerspective();
            }
        }
        if (m_playing)
            emit frameUpdated(frameid);
    }
    return frameid;
}

void Zenovis::doFrameUpdate()
{
    int frameid = getCurrentFrameId();

    //todo: move out of the optix working thread.
#if 0
    ZenoMainWindow* pMainWin = zenoApp->getMainWindow();
    if (!pMainWin)
        return;

    ZTimeline* timeline = pMainWin->timeline();
    if (!timeline)
        return;

    int ui_frameid = timeline->value();
    zenoApp->getMainWindow()->doFrameUpdate(ui_frameid);
#endif

    if (m_playing) {
        zeno::log_trace("playing at frame {}", frameid);
    }
    //zenvis::auto_gc_frame_data(m_cache_frames);

    bool inserted = session->load_objects();
    if (inserted) {
        emit objectsUpdated(frameid);
    }

    if (m_currentCameraName != "none")
    {
        auto scene = session->get_scene();
        scene->renderMan->getEngine()->skipCameraObj(false);
        for (auto const &[key, ptr] : scene->objectsMan->pairs()) {
            if (key.substr(0, key.find(":")) == m_currentCameraName.toStdString()) {
                auto cam = dynamic_cast<zeno::CameraObject *>(ptr)->get();
                scene->camera->setCamera(cam);
            }
        }
    }
    else {
        auto scene = session->get_scene();
        scene->renderMan->getEngine()->skipCameraObj(true);
        //scene->camera->m_need_sync = false;
    }

    if (m_playing)
        setCurrentFrameId(frameid + 1);
}

/*
QList<Zenovis::FRAME_FILE> Zenovis::getFrameFiles(int frameid)
{
    QList<Zenovis::FRAME_FILE> framefiles;
    if (g_iopath.isEmpty())
        return framefiles;

    QString dirPath = QString("%1/%2").arg(g_iopath).arg(QString::number(frameid), 6, QLatin1Char('0'));
    QDir frameDir(dirPath);
    if (!frameDir.exists("done.lock"))
        return framefiles;

    frameDir.setFilter(QDir::Files);

    foreach(QFileInfo fileInfo, frameDir.entryInfoList())
    {
        QString fn = fileInfo.fileName();
        QString path = QString("%1/%2").arg(dirPath).arg(fn);
        QString ext = QString(".") + fileInfo.suffix();
        framefiles.append(FRAME_FILE(fn, ext, path));
    }
    return framefiles;
}*/
