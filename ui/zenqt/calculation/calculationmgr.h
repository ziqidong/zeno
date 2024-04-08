#ifndef __CALCULATION_MGR_H__
#define __CALCULATION_MGR_H__

#include <QObject>
#include <QThread>
#include <string>
#include <QtWidgets>
#include "uicommon.h"


class DisplayWidget;

class CalcWorker : public QObject
{
    Q_OBJECT
public:
    CalcWorker(QObject* parent = nullptr);

signals:
    void calcFinished(bool, zeno::ObjPath, QString);
    void nodeStatusChanged(zeno::ObjPath, NodeState);

public slots:
    void run();

private:
    bool m_bReportNodeStatus = true;    //����������ģʽ�£��Ƿ���ÿ���ڵ������״̬��ǰ��
};


class CalculationMgr : public QObject
{
    Q_OBJECT
public:
    CalculationMgr(QObject* parent);
    void run();
    void kill();
    void registerRenderWid(DisplayWidget* pDisp);
    void unRegisterRenderWid(DisplayWidget* pDisp);

signals:
    void calcFinished(bool, zeno::ObjPath, QString);
    void nodeStatusChanged(zeno::ObjPath, NodeState);

private slots:
    void onCalcFinished(bool, zeno::ObjPath, QString);
    void onNodeStatusReported(zeno::ObjPath, NodeState);
    void on_render_objects_loaded();

private:
    bool m_bMultiThread;
    CalcWorker* m_worker;
    QThread m_thread;
    QSet<DisplayWidget*> m_registerRenders;
    QSet<DisplayWidget*> m_loadedRender;
};

#endif