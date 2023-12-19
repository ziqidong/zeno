#ifndef __COMMAND_NODE_H__
#define __COMMAND_NODE_H__

#include "zenonode.h"
#include <zenomodel/include/enum.h>

class CommandNode : public ZenoNode
{
    struct CMDVALUE
    {
        ZVARIANT zvalue;
        SocketType socketType;
    };

    Q_OBJECT
public:
    CommandNode(const NodeUtilParam& params, QGraphicsItem* parent = nullptr);
    ~CommandNode();

protected:
    ZGraphicsLayout* initCustomParamWidgets() override;
    Callback_OnButtonClicked registerButtonCallback(const QModelIndex& paramIdx) override;

private slots:
    void onExecuteClicked();
    void onGenerateClicked();

private:
    bool parseKeyValue(std::map<std::string, CMDVALUE>& dats, std::string strToParse);
    bool addNode(std::string cmd, std::pair<float, float>& nodepos);
    bool deleteNode(std::string cmd);
    bool addLink(std::string cmd);
    bool setView(std::string cmd);
    bool setValue(std::string cmd);
    std::map<std::string, ZENO_HANDLE>nodeVars;
};

#endif