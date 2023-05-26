#include <zeno/core/Graph.h>
#include <zeno/core/INode.h>
#include <zeno/core/IObject.h>
#include <zeno/core/Session.h>
#include <zeno/utils/safe_at.h>
#include <zeno/utils/scope_exit.h>
#include <zeno/core/Descriptor.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/StringObject.h>
#include <zeno/extra/GraphException.h>
#include <zeno/funcs/LiterialConverter.h>
#include <zeno/extra/GlobalStatus.h>
#include <zeno/extra/SubnetNode.h>
#include <zeno/utils/Error.h>
#include <zeno/utils/log.h>
#include <iostream>

namespace zeno {

ZENO_API Context::Context() = default;
ZENO_API Context::~Context() = default;

ZENO_API Context::Context(Context const &other)
    : visited(other.visited)
{}

ZENO_API Graph::Graph() = default;
ZENO_API Graph::~Graph() = default;

ZENO_API zany const &Graph::getNodeOutput(
    std::string const &sn, std::string const &ss) const {
    auto node = safe_at(nodes, sn, "node name").get();
    if (node->muted_output)
        return node->muted_output;
    return safe_at(node->outputs, ss, "output socket name of node " + node->myname);
}

ZENO_API void Graph::clearNodes() {
    nodes.clear();
}

ZENO_API void Graph::addNode(std::string const &cls, std::string const &id) {
    if (nodes.find(id) != nodes.end())
        return;  // no add twice, to prevent output object invalid
    auto cl = safe_at(session->nodeClasses, cls, "node class name").get();
    auto node = cl->new_instance();
    node->graph = this;
    node->myname = id;
    node->nodeClass = cl;
    nodes[id] = std::move(node);
}

ZENO_API Graph *Graph::addSubnetNode(std::string const &id) {
    auto subcl = std::make_unique<ImplSubnetNodeClass>();
    auto node = subcl->new_instance();
    node->graph = this;
    node->myname = id;
    node->nodeClass = subcl.get();
    auto subnode = static_cast<SubnetNode *>(node.get());
    subnode->subgraph->session = this->session;
    subnode->subnetClass = std::move(subcl);
    auto subg = subnode->subgraph.get();
    nodes[id] = std::move(node);
    return subg;
}

ZENO_API Graph *Graph::getSubnetGraph(std::string const &id) const {
    auto node = static_cast<SubnetNode *>(safe_at(nodes, id, "node name").get());
    return node->subgraph.get();
}

ZENO_API void Graph::completeNode(std::string const &id) {
    safe_at(nodes, id, "node name")->doComplete();
}

ZENO_API void Graph::applyNode(std::string const &id) {
    if (ctx->visited.find(id) != ctx->visited.end()) {
        return;
    }
    ctx->visited.insert(id);
    auto node = safe_at(nodes, id, "node name").get();
    GraphException::translated([&] {
        node->doApply();
    }, node->myname);
}

ZENO_API void Graph::applyNodes(std::set<std::string> const &ids) {
    ctx = std::make_unique<Context>();

    scope_exit _{[&] {
        ctx = nullptr;
    }};

    for (auto const &id: ids) {
        applyNode(id);
    }
}

ZENO_API void Graph::applyNodesToExec() {
    log_debug("{} nodes to exec", nodesToExec.size());
    applyNodes(nodesToExec);
}

ZENO_API void Graph::bindNodeInput(std::string const &dn, std::string const &ds,
        std::string const &sn, std::string const &ss) {
    safe_at(nodes, dn, "node name")->inputBounds[ds] = std::pair(sn, ss);
}

ZENO_API void Graph::setNodeInput(std::string const &id, std::string const &par,
        zany const &val) {
    safe_at(nodes, id, "node name")->inputs[par] = val;
}

ZENO_API std::map<std::string, zany> Graph::callSubnetNode(std::string const &id,
        std::map<std::string, zany> inputs) const {
    auto se = safe_at(nodes, id, "node name").get();
    se->inputs = std::move(inputs);
    se->doOnlyApply();
    return std::move(se->outputs);
}

ZENO_API std::map<std::string, zany> Graph::callTempNode(std::string const &id,
        std::map<std::string, zany> inputs) const {
    auto cl = safe_at(session->nodeClasses, id, "node class name").get();
    auto se = cl->new_instance();
    se->graph = const_cast<Graph *>(this);
    se->inputs = std::move(inputs);
    se->doOnlyApply();
    return std::move(se->outputs);
}

ZENO_API void Graph::addNodeOutput(std::string const& id, std::string const& par) {
    // add "dynamic" output which is not descriped by core.
    safe_at(nodes, id, "node name")->outputs[par] = nullptr;
}

ZENO_API void Graph::setNodeInputFormula(std::string const &id, std::string const &par, Value const &val) {
    auto callTmpNode = [&](std::string value) {
        std::shared_ptr<IObject> iobj = callTempNode("NumericEval", {{"zfxCode", objectFromLiterial(value)},
                                         {"resType", objectFromLiterial("float")}})["result"];
        std::shared_ptr<NumericObject> res = std::dynamic_pointer_cast<NumericObject>(iobj);
        return res;
    };
    if (val.IsArray())
    {
        auto a = val.GetArray();
        if (a.Size() == 2)
        {
            float res[2];
            for (int i = 0; i < a.Size(); i++) {
                res[i] = objectToLiterial<float>(callTmpNode(a[i].GetString()));
            }
            setNodeInput(id, par, objectFromLiterial(vec2f(res[0], res[1])));
        }else if (a.Size() == 3)
        {
            float res[3];
            for (int i = 0; i < a.Size(); i++) {
                res[i] = objectToLiterial<float>(callTmpNode(a[i].GetString()));
            }
            setNodeInput(id, par, objectFromLiterial(vec3f(res[0], res[1], res[2])));
        }else if (a.Size() == 4)
        {
            float res[4];
            for (int i = 0; i < a.Size(); i++) {
                res[i] = objectToLiterial<float>(callTmpNode(a[i].GetString()));
            }
            setNodeInput(id, par, objectFromLiterial(vec4f(res[0], res[1], res[2], res[3])));
        }
    }
    else if (val.IsString())
    {
        zany res = callTempNode("NumericEval", {{"zfxCode", objectFromLiterial(val.GetString())},
                                                {"resType", objectFromLiterial("float")}})["result"];
        setNodeInput(id, par, res);
    }
}

ZENO_API void Graph::setNodeParam(std::string const &id, std::string const &par,
    std::variant<int, float, std::string, zany> const &val) {
    auto parid = par + ":";
    std::visit([&] (auto const &val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, zany>) {
            setNodeInput(id, parid, val);
        } else {
            setNodeInput(id, parid, objectFromLiterial(val));
        }
    }, val);
}

}
