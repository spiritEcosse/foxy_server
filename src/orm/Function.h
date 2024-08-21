namespace api::v1 {

    class Function {
    public:
        explicit Function() = default;

        explicit Function(std::string function) : function(std::move(function)) {}

        [[nodiscard]] std::string toStr() const {
            return function;
        }

        [[nodiscard]] std::string getFullFieldName() const {
            return function;
        }

        std::string function;
    };
}