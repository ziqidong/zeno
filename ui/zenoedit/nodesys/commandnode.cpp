#include "CommandNode.h"
#include <zeno/extra/TempNode.h>
#include "launch/corelaunch.h"
#include "zenoapplication.h"
#include "zenomainwindow.h"
#include <zenomodel/include/graphsmanagment.h>
#include <zenomodel/include/igraphsmodel.h>
#include <regex>
#include <zenomodel/include/api.h>


CommandNode::CommandNode(const NodeUtilParam& params, QGraphicsItem* parent)
    : ZenoNode(params, parent)
{
}

CommandNode::~CommandNode()
{
}

Callback_OnButtonClicked CommandNode::registerButtonCallback(const QModelIndex& paramIdx)
{
    //todo: check whether there is commands input.
    if (paramIdx.data(ROLE_PARAM_NAME) == "source")
    {
        Callback_OnButtonClicked cb = [=]() {
            onGenerateClicked();
        };
        return cb;
    }
    return ZenoNode::registerButtonCallback(paramIdx);
}

ZGraphicsLayout* CommandNode::initCustomParamWidgets()
{
    ZGraphicsLayout* pHLayout = new ZGraphicsLayout(true);

    ZenoParamPushButton* pExecuteBtn = new ZenoParamPushButton("Execute", -1, QSizePolicy::Expanding);
    pExecuteBtn->setMinimumHeight(28);
    pHLayout->addSpacing(-1);
    pHLayout->addItem(pExecuteBtn);
    pHLayout->addSpacing(-1);
    connect(pExecuteBtn, SIGNAL(clicked()), this, SLOT(onExecuteClicked()));
    return pHLayout;
}

void CommandNode::onGenerateClicked()
{
    auto main = zenoApp->getMainWindow();
    ZASSERT_EXIT(main);

    IGraphsModel* pModel = zenoApp->graphsManagment()->currentModel();

    TIMELINE_INFO tinfo = main->timelineInfo();

    LAUNCH_PARAM launchParam;
    launchParam.beginFrame = tinfo.beginFrame;
    launchParam.endFrame = tinfo.endFrame;
    QString path = pModel->filePath();
    path = path.left(path.lastIndexOf("/"));
    launchParam.zsgPath = path;

    launchParam.projectFps = main->timelineInfo().timelinefps;
    launchParam.generator = this->nodeId();

    AppHelper::initLaunchCacheParam(launchParam);
    launchProgram(pModel, launchParam);
}

bool CommandNode::parseKeyValue(std::map<std::string, CMDVALUE>& dats, std::string strToParse)
{
    std::string pattern1 = "\\\{.*?\\\}.*?,";           //match content "{...}"
    std::string pattern2 = "\".*?\".*?,";               //match content "..."
    std::string pattern3 = "-?[0-9]+(\.[0-9]+)?.*?,";   //match integer and decimal
    std::string pattern4 = "true|false.*?,";            //match bool
    //std::regex r(pattern1 + "|" + pattern2 + "|" + pattern3 + "|" + pattern4 + "|" + pattern5);
    std::regex r(pattern1 + "|" + pattern2 + "|" + pattern3 + "|" + pattern4);
    std::sregex_iterator iter(strToParse.begin(), strToParse.end(), r);
    std::sregex_iterator end;

    bool ret = true;
    bool oddNum = false;
    std::string key;
    CMDVALUE value;
    while (iter != end)
    {
        std::string iterStr = ((std::smatch)*iter).str();
        ZVARIANT content;
        SocketType type;
        if (std::regex_match(iterStr, std::regex("\".*\".*?,")))                    //"..." string
        {
            std::smatch match;
            if (std::regex_search(iterStr, match, std::regex("\"(.*)\".*?,")))
                if (match.size() == 2) {
                    content = match[1].str();
                    type = ST_STRING;
                }
        }
        else if (std::regex_match(iterStr, std::regex("\\\{.*\\\}.*?,")))          //{...} vec
        {
            std::smatch match;
            if (std::regex_search(iterStr, match, std::regex("\\\{(.*)\\\}.*?,")))
                if (match.size() == 2) {
                    bool isFloat = false;
                    std::stringstream ss(match[1].str());
                    std::vector<std::string> result;
                    std::string token;
                    while (std::getline(ss, token, ',')) {
                        token.erase(std::remove(token.begin(), token.end(), ' '), token.end());
                        if (std::regex_match(token, std::regex("-?[0-9]+(\.[0-9]+)")))
                            isFloat = true;
                        result.push_back(token);
                    }
                    try {
                        if (result.size() == 2)
                            if (isFloat) {
                                content = zeno::vec2f(std::stof(result[0]), std::stof(result[1]));
                                type = ST_VEC2F;
                            }
                            else {
                                content = zeno::vec2i(std::stoi(result[0]), std::stoi(result[1]));
                                type = ST_VEC2I;
                            }
                        else if (result.size() == 3)
                            if (isFloat) {
                                content = zeno::vec3f(std::stof(result[0]), std::stof(result[1]), std::stof(result[2]));
                                type = ST_VEC3F;
                            }
                            else {
                                content = zeno::vec3i(std::stoi(result[0]), std::stoi(result[1]), std::stoi(result[2]));
                                type = ST_VEC3I;
                            }
                        else if (result.size() == 4)
                            if (isFloat) {
                                content = zeno::vec4f(std::stof(result[0]), std::stof(result[1]), std::stof(result[2]), std::stof(result[3]));
                                type = ST_VEC4F;
                            }
                            else {
                                content = zeno::vec4i(std::stoi(result[0]), std::stoi(result[1]), std::stoi(result[2]), std::stoi(result[3]));
                                type = ST_VEC4I;
                            }
                    }
                    catch (std::invalid_argument const& ex)
                    {
                        zeno::log_error("std::invalid_argument: {} ", "{" + result[0] + "," + result[1] + (result.size() == 2 ? "" : result.size() == 3 ? "," + result[2] : "," + result[2] + "," + result[3]) + "}");
                    }
                    catch (std::out_of_range const& ex)
                    {
                        zeno::log_error("std::out_of_range: {} ", "{" + result[0] + "," + result[1] + (result.size() == 2 ? "" : result.size() == 3 ? "," + result[2] : "," + result[2] + "," + result[3]) + "}");
                    }
                }
        }
        else if (std::regex_match(iterStr, std::regex("-?[0-9]+(\.[0-9]+)?.*?,"))) {    //integer, decimal
            std::smatch match;
            if (std::regex_search(iterStr, match, std::regex("-?[0-9]+(\.[0-9]+)?.*?,")))
                if (match.size() == 2) {
                    try
                    {
                        if (std::regex_match(match[0].str(), std::regex("-?[0-9]+(\.[0-9]+).*?,"))) {
                            content = std::stof(match[0].str());
                            type = ST_FLOAT;
                        }
                        else {
                            content = std::stoi(match[0].str());
                            type = ST_INT;
                        }
                    }
                    catch (std::invalid_argument const& ex)
                    {
                        zeno::log_error("std::invalid_argument: {} ", match[0].str());
                    }
                    catch (std::out_of_range const& ex)
                    {
                        zeno::log_error("std::out_of_range: {} ", match[0].str());
                    }
                }
        }
        else if (std::regex_match(iterStr, std::regex("true|false.*?,")))          //bool
        {
            std::smatch match;
            if (std::regex_search(iterStr, match, std::regex("true|false"))) {
                content = match[0].str() == "false" ? false : true;
                type = ST_BOOL;
            }
        }
        oddNum = !oddNum;
        if (oddNum) {
            if (type != ST_STRING)
            {
                zeno::log_error("key is not String: {}", iterStr);
                return false;
            }
            key = std::get<std::string>(content);
        }
        else {
            value.zvalue = content;
            value.socketType = type;
            dats.insert(std::make_pair(key, value));
        }
        ++iter;
    }
    return ret;
}

bool CommandNode::addNode(std::string cmd, std::pair<float, float>& nodepos)
{
    std::vector<std::string> result;
    if (std::regex_match(cmd, std::regex(".*=.*")))
    {
        std::stringstream ss(cmd);
        std::string token;
        while (std::getline(ss, token, '=')) {
            auto begin = token.find_first_not_of(" ");
            auto end = token.find_last_not_of(" ");
            token = token.substr(begin, end - begin + 1);
            result.push_back(token);
        }
        cmd = result[1];
    }

    std::map<std::string, CMDVALUE> dats;
    std::smatch match;
    if (std::regex_search(cmd, match, std::regex("AddNode\\((.*?)\\)"))) {
        if (match.size() == 2) {
            if (!parseKeyValue(dats, cmd + ",")) {
                zeno::log_info("command parse fail: {}.", cmd);
                return false;
            }
        }
    }
    else {
        zeno::log_info("unknow command: {}.", cmd);
        return false;
    }
    if (dats.find("main") == dats.end())
    {
        zeno::log_error("can only create node in main graph.");
        return false;
    }
    if (result.size() != 0 && nodeVars.find(result[0]) != nodeVars.end())
    {
        ZENO_HANDLE hGraph = Zeno_GetGraph("main");
        IGraphsModel* pModel = GraphsManagment::instance().currentModel();
        if (!pModel)
            return false;
        QModelIndex idx = pModel->nodeIndex(hGraph, nodeVars[result[0]]);
        if (!idx.isValid())
            return false;
        nodeVars.insert(std::make_pair("#" + idx.data(ROLE_OBJID).toString().toStdString(), nodeVars[result[0]]));
        nodeVars.erase(result[0]);
    }

    ZVARIANT newNodeName = dats["main"].zvalue;
    dats.erase("main");
    IGraphsModel* pModel = GraphsManagment::instance().currentModel();
    NODE_DESC desc;
    if (!pModel->getDescriptor(QString::fromStdString(std::get<std::string>(newNodeName)), desc))
    {
        zeno::log_info("can not create node: {}.", std::get<std::string>(newNodeName));
        return false;
    }

    ZENO_HANDLE hGraph = Zeno_GetGraph("main");
    ZENO_HANDLE newNode = Zeno_AddNode(hGraph, std::get<std::string>(newNodeName));
    Zeno_SetPos(hGraph, newNode, nodepos);
    if (!result.empty())
    {
        nodeVars.insert(std::make_pair(result[0], newNode));
    }
    if (!pModel)
        return false;
    QModelIndex idx = pModel->nodeIndex(hGraph, newNode);
    if (!idx.isValid())
        return false;
    INPUT_SOCKETS inputs = idx.data(ROLE_INPUTS).value<INPUT_SOCKETS>();
    for (auto& [k, v] : dats)
    {
        const QString& qsInSock = QString::fromStdString(k);
        if (inputs.find(qsInSock) == inputs.end()) {
            zeno::log_info("socket not exist: {}.", k);
            return false;
        }
        //INPUT_SOCKET input = inputs[qsInSock];    //type check
        //if (input.info.control == CONTROL_STRING && ((CMDVALUE)v).socketType != ST_STRING ||
        //    input.info.control == CONTROL_VEC2_INT && ((CMDVALUE)v).socketType != ST_VEC2I ||
        //    input.info.control == CONTROL_VEC2_FLOAT && ((CMDVALUE)v).socketType != ST_VEC2F ||
        //    input.info.control == CONTROL_VEC3_INT && ((CMDVALUE)v).socketType != ST_VEC3I ||
        //    input.info.control == CONTROL_VEC3_FLOAT && ((CMDVALUE)v).socketType != ST_VEC3F ||
        //    input.info.control == CONTROL_VEC4_INT && ((CMDVALUE)v).socketType != ST_VEC4I ||
        //    input.info.control == CONTROL_VEC4_FLOAT && ((CMDVALUE)v).socketType != ST_VEC4F ||
        //    input.info.control == CONTROL_FLOAT && ((CMDVALUE)v).socketType != ST_FLOAT ||
        //    input.info.control == CONTROL_INT && ((CMDVALUE)v).socketType != ST_INT ||
        //    input.info.control == CONTROL_BOOL && ((CMDVALUE)v).socketType != ST_BOOL
        //    )
        //{
        //    zeno::log_info("socket type error: {}.", k);
        //    return false;
        //}
        if (Zeno_SetInputDefl(hGraph, newNode, k, (ZVARIANT)v.zvalue) != Err_NoError)
        {
            zeno::log_info("set default value fail: key:{}.", k);
            return false;
        }
    }
    return true;
}

bool CommandNode::deleteNode(std::string cmd)
{
    cmd.erase(std::remove(cmd.begin(), cmd.end(), ' '), cmd.end());
    if (cmd.size() == 0)
    {
        zeno::log_info("empty input.");
        return false;
    }
    ZENO_HANDLE hGraph = Zeno_GetGraph("main");
    if (nodeVars.find(cmd) == nodeVars.end())
    {
        zeno::log_info("node not found: {}.", cmd);
        return false;
    }
    if (Zeno_DeleteNode(hGraph, nodeVars[cmd]) != Err_NoError)
    {
        zeno::log_info("delete node fail: {}.", cmd);
        return false;
    }
}

bool CommandNode::addLink(std::string cmd)
{
    const auto& parseCmd = [&](std::string strToParse) {
        strToParse.erase(std::remove(strToParse.begin(), strToParse.end(), ' '), strToParse.end());
        std::stringstream ss(strToParse);
        std::vector<std::string> result;
        std::string token;
        while (std::getline(ss, token, ',')) {
            result.push_back(token);
        }
        if (result.size() != 4)
        {
            return false;
        }
        std::string& node1 = result[0];
        std::string& node2 = result[2];
        std::string& sock1 = result[1];
        std::string& sock2 = result[3];
        std::smatch match;
        if (std::regex_search(sock1, match, std::regex("\"(.*)\""))) {
            if (match.size() == 2)
                sock1 = match[1].str();
        }
        else {
            zeno::log_info("param invalid.", sock1);
            return false;
        }
        if (std::regex_search(sock2, match, std::regex("\"(.*)\""))) {
            if (match.size() == 2)
                sock2 = match[1].str();
        }
        else {
            zeno::log_info("param invalid.", sock1);
            return false;
        }
        if (nodeVars.find(node1) == nodeVars.end())
        {
            zeno::log_info("param invalid.", node1);
            return false;
        }
        if (nodeVars.find(node2) == nodeVars.end())
        {
            zeno::log_info("param invalid.", node2);
            return false;
        }
        if (Zeno_AddLink(Zeno_GetGraph("main"), nodeVars[node1], result[1], nodeVars[node2], result[3]) != Err_NoError)
        {
            zeno::log_info("add link fail: {}:{}--{}:{}.", node1, result[1], node2, result[3]);
            return false;
        }
        return true;
    };
    if (!parseCmd(cmd)) {
        zeno::log_info("command parse fail: {}.", cmd);
        return false;
    }
    return true;
}

bool CommandNode::setView(std::string cmd)
{
    cmd.erase(std::remove(cmd.begin(), cmd.end(), ' '), cmd.end());
    std::stringstream ss(cmd);
    std::vector<std::string> result;
    std::string token;
    while (std::getline(ss, token, ',')) {
        result.push_back(token);
    }
    if (result.size() == 0)
    {
        zeno::log_info("command parse fail: {}.", cmd);
        return false;
    }
    std::string& node = result[0];
    bool enable;
    if (std::regex_match(result[1], std::regex("true|false")))
        enable = result[1] == "false" ? false : true;
    else {
        zeno::log_info("invalid param: {}.", result[1]);
        return false;
    }

    if (Zeno_SetView(Zeno_GetGraph("main"), nodeVars[node], enable) != Err_NoError)
    {
        zeno::log_info("set view on fail: {}.", node);
        return false;
    }
    //if (enable && Zeno_SetCache(Zeno_GetGraph("main"), nodeVars[node], enable) != Err_NoError) {
    //    zeno::log_info("set cache on fail: {}.", node);
    //    return false;
    //}
}

bool CommandNode::setValue(std::string cmd)
{
    std::string node;
    std::smatch match;
    if (std::regex_search(cmd, match, std::regex("\\\s*(.*?)\\\s*,"))) {
        if (match.size() == 2) {
            node = match[1].str();
        }
    }
    if (node.size() == 0)
    {
        zeno::log_info("command parse fail: {}.", cmd);
        return false;
    }
    if (nodeVars.find(node) == nodeVars.end())
    {
        zeno::log_info("command parse fail: {}.", cmd);
        return false;
    }
    std::map<std::string, CMDVALUE> dats;
    if (!parseKeyValue(dats, cmd.substr(cmd.find(",") + 1) + ",")) {
        zeno::log_info("command parse fail: {}.", cmd);
        return false;
    }

    ZENO_HANDLE hGraph = Zeno_GetGraph("main");
    IGraphsModel* pModel = GraphsManagment::instance().currentModel();
    if (!pModel)
        return false;
    QModelIndex idx = pModel->nodeIndex(hGraph, nodeVars[node]);
    if (!idx.isValid())
        return false;
    for (auto& [k, v] : dats)
    {
        INPUT_SOCKETS inputs = idx.data(ROLE_INPUTS).value<INPUT_SOCKETS>();
        const QString& qsInSock = QString::fromStdString(k);
        if (inputs.find(qsInSock) == inputs.end()) {
            zeno::log_info("socket not exist: {}.", k);
            return false;
        }
        //type check
        //...
        if (Zeno_SetInputDefl(hGraph, nodeVars[node], k, (ZVARIANT)v.zvalue) != Err_NoError)
        {
            zeno::log_info("set default value fail: node:{}, key:{}.", node, k);
            return false;
        }
    }
    return true;
}

void CommandNode::onExecuteClicked()
{
    //clear last generated nodes
    for (auto& [k, v]: nodeVars)
    {
        ZENO_HANDLE hGraph = Zeno_GetGraph("main");
        Zeno_DeleteNode(hGraph, v);
    }
    nodeVars.clear();

    //re caculate nodes pos
    ZENO_HANDLE hGraph = Zeno_GetGraph("main");
    std::pair<float, float> cmdNodePos;
    Zeno_GetPos(hGraph, index().internalId(), cmdNodePos);
    int offsetX = 0;

    //parse
    IGraphsModel* pModel = zenoApp->graphsManagment()->currentModel();
    QModelIndex subgIdx = pModel->index("main");
    QModelIndex commandsIdx = pModel->paramIndex(subgIdx, this->index(), "commands", true);
    ZASSERT_EXIT(commandsIdx.isValid());
    QString commands = commandsIdx.data(ROLE_PARAM_VALUE).toString();
    QStringList cmds = commands.split("\n", Qt::SkipEmptyParts);
    for (QString cmd : cmds) {
        std::string stdCmd = cmd.trimmed().toStdString();

        if (stdCmd.find_last_of('\0') != std::string::npos) //remove \0 in the end
            stdCmd.erase(stdCmd.find('\0'), 1);

        if (std::regex_match(stdCmd, std::regex(".*=?.*AddNode\\(.*\\)")))
        {
            cmdNodePos = { cmdNodePos.first + 600, cmdNodePos.second };
            addNode(stdCmd, cmdNodePos);
        }
        else if (std::regex_match(stdCmd, std::regex("AddLink\\(.*\\)")))
        {
            std::smatch match;
            if (std::regex_search(stdCmd, match, std::regex("AddLink\\((.*?)\\)"))) {
                if (match.size() == 2) {
                    addLink(match[1].str());
                }
            }
        }
        else if (std::regex_match(stdCmd, std::regex("SetView\\(.*\\)")))
        {
            std::smatch match;
            if (std::regex_search(stdCmd, match, std::regex("SetView\\((.*?)\\)"))) {
                if (match.size() == 2) {
                    setView(match[1].str());
                }
            }
        }else if (std::regex_match(stdCmd, std::regex("SetValue\\(.*\\)")))
        {
            std::smatch match;
            if (std::regex_search(stdCmd, match, std::regex("SetValue\\((.*?)\\)"))) {
                if (match.size() == 2) {
                    setValue(match[1].str());
                }
            }
        }else if (std::regex_match(stdCmd, std::regex("DeleteNode\\(.*\\)")))
        {
            std::smatch match;
            if (std::regex_search(stdCmd, match, std::regex("DeleteNode\\((.*?)\\)"))) {
                if (match.size() == 2) {
                    deleteNode(match[1].str());
                    //if (!parseKeyValue(dats, match[1].str() + ",")) {
                    //    zeno::log_info("command parse fail: {}.", stdCmd);
                    //    return;
                    //}
                }
            }
        }else {
            zeno::log_error("unknow command: {}.", stdCmd);
        }

        //api examples:
        // NewGraph("graphName")
        // RemoveGraph("graphName")
        // idA = AddNode("graphName", "LightNode", "<preset-id>")  preset-id is a ident generated by generator.
        // AddLink("graphName", "xxx-LightNode", "outparamname", "yyy-LightNode", "inparamname")
    }
}
