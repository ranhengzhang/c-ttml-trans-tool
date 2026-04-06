//
// Created by LEGION on 2026/4/6.
//

#ifndef LYRIC_PARSER_AGENT_H
#define LYRIC_PARSER_AGENT_H

#include <QStringList>

#include <map>

#include "utils.hpp"

namespace lyric::utils {
    class Agent {
    public:
        enum class AgentType {
            Person,
            Group,
            Other
        };

        [[nodiscard]] AgentType getType() const;

        [[nodiscard]] QString getId() const;

        [[nodiscard]] bool isPerson() const {return this->_type == AgentType::Person;}

        [[nodiscard]] bool isGroup() const {return this->_type == AgentType::Group;}

        [[nodiscard]] static std::pair<Agent, Status> fromTTML(const QDomElement &xml);

        [[nodiscard]] QString toTTML();

    private:
        static std::map<AgentType, QString> _type_name;
        static std::map<QString, AgentType> _type_enum;

        AgentType _type{};
        QString _id{};
        QStringList _names{};
    };
}

#endif //LYRIC_PARSER_AGENT_H
