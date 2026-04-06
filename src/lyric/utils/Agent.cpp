//
// Created by LEGION on 2026/4/6.
//

#include "Agent.hpp"

namespace lyric::utils {
    std::map<Agent::AgentType, QString> Agent::_type_name = {
        {Agent::AgentType::Person, "person"},
        {Agent::AgentType::Group, "group"},
        {Agent::AgentType::Other, "other"}
    };

    std::map<QString, Agent::AgentType> Agent::_type_enum = {
        {"person", Agent::AgentType::Person},
        {"group", Agent::AgentType::Group},
        {"other", Agent::AgentType::Other}
    };

    Agent::AgentType Agent::getType() const {
        return this->_type;
    }

    QString Agent::getId() const {
        return this->_id;
    }

    std::pair<Agent, Status> Agent::fromTTML(const QDomElement &xml) {
        const auto el = xml.toElement();

        if (not el.hasAttribute("type") or not el.hasAttribute("xml:id")) {
            return {{}, Status::InvalidStructure};
        }

        Agent agent;
        const auto type = el.attribute("type").toLower();
        const auto id = el.attribute("xml:id");

        if (not _type_enum.contains(type)) {
            return {{}, Status::InvalidStructure};
        }

        agent._type = _type_enum[type];
        agent._id = id;

        const auto name_s = el.elementsByTagName("ttm:name");
        for (int i = 0; i < name_s.count(); i++) {
            const auto name = name_s.at(i).toElement().text();
            agent._names.append(name);
        }

        return {agent, Status::Success};
    }

    QString Agent::toTTML() {
        QStringList agent_names{};

        for (const auto &name: this->_names) {
            agent_names.append(QString(R"(<ttm:name type="full">%1</ttm:name>)").arg(toHtmlEscaped(name)));
        }

        return QString(R"(<ttm:agent type="%1" xml:id="%2"%3)")
            .arg(_type_name[this->_type])
            .arg(this->_id)
            .arg(agent_names.isEmpty() ? R"(/>)" : QString(R"(>%1</ttm:agent>)").arg(agent_names.join("")));
    }
}
