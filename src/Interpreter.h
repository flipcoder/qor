#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <boost/python.hpp>
#include <string>
#include <vector>

class Interpreter
{
    public:

        Interpreter(
            const char* name = "",
            void* userdata = nullptr,
            std::vector<std::string> paths = std::vector<std::string>() 
        );
        ~Interpreter();

        class Context {
            public:
                Context(Interpreter* interp);
                ~Context();
                void clear();
                void execute_file(const std::string& fn);
                void execute_string(const std::string& code);
                boost::python::object evaluate_string(const std::string& code);
                Interpreter* interpreter() {
                    return m_pInterpreter;
                }
            private:
                Interpreter* m_pInterpreter;
                boost::python::object m_Main;
                boost::python::object m_Global;
                std::vector<std::string> m_Paths;
        };

        class Selection {
            public:
                Selection(
                    Context* context
                ){
                    Interpreter::select(context);
                }
                ~Selection() {
                    Interpreter::deselect();
                }
        };
        
        void* user_data() {
            return m_pUserData;
        }
        static Interpreter* current() {
            return s_Current.back()->interpreter();
        }
        static Context* context() {
            return s_Current.back();
        }

        static void deselect() {
            s_Current.pop_back();
        }
        static void select(
            Context* context
        ) {
            s_Current.push_back(context);
        }

        std::vector<std::string> paths() const {
            return m_Paths;
        }
        
    private:
        
        void* m_pUserData;
        static std::vector<Interpreter::Context*> s_Current;
        std::vector<std::string> m_Paths;
};

#endif

