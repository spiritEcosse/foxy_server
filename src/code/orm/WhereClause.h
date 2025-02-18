#pragma once

#include "BaseClass.h"
#include "BaseField.h"

#include <string>
#include <optional>
#include <memory>
#include <vector>
#include <sstream>
#include <iostream>
#include <fmt/format.h>

enum class Operator { EQUALS, NOT_EQUALS, GREATER_THAN, LESS_THAN, GREATER_OR_EQUAL, LESS_OR_EQUAL, LIKE, IS };

inline std::string operatorToString(const Operator op) {
    using enum Operator;

    switch(op) {
        case EQUALS:
            return "=";
        case NOT_EQUALS:
            return "!=";
        case GREATER_THAN:
            return ">";
        case LESS_THAN:
            return "<";
        case GREATER_OR_EQUAL:
            return ">=";
        case LESS_OR_EQUAL:
            return "<=";
        case LIKE:
            return "LIKE";
        case IS:
            return "IS";
        default:
            throw std::invalid_argument("Unknown operator");
    }
}

inline std::string boolToString(const bool value) {
    return value ? "TRUE" : "FALSE";
}

namespace api::v1 {
    class WhereClause final : public BaseClass {
    public:
        enum class LogicalType { SINGLE, AND, OR };
        enum class ValueType { STRING, SPECIAL, FIELD_COMPARISON };

        WhereClause(const BaseField* field, std::string value, const Operator op = Operator::EQUALS) :
            BaseClass(), _field(field), _value(std::move(value)), _op(op), _valueType(ValueType::STRING) {}

        WhereClause(const BaseField* field, const std::optional<bool> value, const Operator op = Operator::EQUALS) :
            BaseClass(), _field(field), _value(value ? boolToString(value.value()) : "NULL"), _op(op),
            _valueType(ValueType::SPECIAL) {}

        WhereClause(const BaseField* field1, const BaseField* field2) :
            BaseClass(), _field(field1), _comparisonField(field2), _op(Operator::EQUALS),
            _valueType(ValueType::FIELD_COMPARISON) {}

        friend WhereClause operator&(WhereClause lhs, WhereClause rhs) {
            WhereClause combined(std::move(lhs));
            combined._type = LogicalType::AND;
            combined._subclauses.push_back(std::move(rhs));
            return combined;
        }

        friend WhereClause operator|(WhereClause lhs, WhereClause rhs) {
            WhereClause combined(std::move(lhs));
            combined._type = LogicalType::OR;
            combined._subclauses.push_back(std::move(rhs));
            return combined;
        }

        std::string serialize() const {
            std::stringstream ss;
            serializeRecursive(ss);
            return ss.str();
        }

    private:
        void serializeRecursive(std::stringstream& ss) const {
            using enum LogicalType;

            const bool needParentheses = _type != SINGLE;

            if(needParentheses)
                ss << "(";

            ss << serializeSingleClause();
            if(_type != SINGLE) {
                ss << (_type == AND ? " AND " : " OR ");

                for(const auto& subClause: _subclauses) {
                    subClause.serializeRecursive(ss);
                }
            }

            if(needParentheses)
                ss << ")";
        }

        std::string serializeSingleClause() const {
            switch(_valueType) {
                case ValueType::FIELD_COMPARISON:
                    return fmt::format("{} {} {}",
                                       _field->getFullFieldName(),
                                       operatorToString(_op),
                                       _comparisonField->getFullFieldName());

                case ValueType::STRING:
                    return fmt::format("{} {} {}",
                                       _field->getFullFieldName(),
                                       operatorToString(_op),
                                       fmt::format("'{}'", _value));

                case ValueType::SPECIAL:
                    return fmt::format("{} {} {}", _field->getFullFieldName(), operatorToString(_op), _value);

                default:
                    return "";
            }
        }

        const BaseField* _field;
        const BaseField* _comparisonField = nullptr;
        std::string _value;
        Operator _op;
        ValueType _valueType;
        LogicalType _type = LogicalType::SINGLE;
        std::vector<WhereClause> _subclauses;
    };
}
