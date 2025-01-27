#pragma once

#include "BaseClass.h"
#include "BaseField.h"

#include <string>
#include <optional>
#include <memory>
#include <vector>
#include <sstream>
#include <fmt/format.h>

enum class Operator { EQUALS, NOT_EQUALS, GREATER_THAN, LESS_THAN, GREATER_OR_EQUAL, LESS_OR_EQUAL, LIKE, IS };

inline std::string operatorToString(const Operator op) {
    switch(op) {
        case Operator::EQUALS:
            return "=";
        case Operator::NOT_EQUALS:
            return "!=";
        case Operator::GREATER_THAN:
            return ">";
        case Operator::LESS_THAN:
            return "<";
        case Operator::GREATER_OR_EQUAL:
            return ">=";
        case Operator::LESS_OR_EQUAL:
            return "<=";
        case Operator::LIKE:
            return "LIKE";
        case Operator::IS:
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

        friend WhereClause operator&(const WhereClause& lhs, const WhereClause& rhs) {
            WhereClause combined(lhs._field, lhs._value, lhs._op);
            combined._valueType = lhs._valueType;
            combined._comparisonField = lhs._comparisonField;
            combined._type = LogicalType::AND;
            combined._subclauses.push_back(std::make_shared<WhereClause>(rhs._field, rhs._value, rhs._op));
            return combined;
        }

        friend WhereClause operator|(const WhereClause& lhs, const WhereClause& rhs) {
            WhereClause combined(lhs._field, lhs._value, lhs._op);
            combined._valueType = lhs._valueType;
            combined._comparisonField = lhs._comparisonField;
            combined._type = LogicalType::OR;
            combined._subclauses.push_back(std::make_shared<WhereClause>(rhs._field, rhs._value, rhs._op));
            return combined;
        }

        // Serialize the clause
        std::string serialize() const {
            std::stringstream ss;
            serializeRecursive(ss);
            return ss.str();
        }

    private:
        void serializeRecursive(std::stringstream& ss) const {
            const bool needParentheses = (_type != LogicalType::SINGLE);

            if(needParentheses)
                ss << "(";

            if(_type == LogicalType::SINGLE) {
                ss << serializeSingleClause();
            } else {
                // First clause (this)
                ss << serializeSingleClause();

                // Add logical operator
                ss << (_type == LogicalType::AND ? " AND " : " OR ");

                // Subclauses
                for(const auto& subClause: _subclauses) {
                    subClause->serializeRecursive(ss);
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
        std::vector<std::shared_ptr<WhereClause>> _subclauses;
    };
}
