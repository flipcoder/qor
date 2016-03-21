#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <boost/python.hpp>
#include <boost/signals2.hpp>
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
                bool execute_file(const std::string& fn);
                bool execute_string(const std::string& code);
                void with(std::function<void()> func);
                boost::python::object evaluate_string(const std::string& code);
                Interpreter* interpreter() {
                    return m_pInterpreter;
                }
                //std::string error() {
                //    std::string r = std::move(m_Error);
                //    return r;
                //}
                boost::signals2::connection on_error(std::function<void(std::string)> cb){
                    return m_onError.connect(cb);
                }
            private:
                void error(std::string err) {
                    //m_Error = err;
                    m_onError(err);
                }
                
                Interpreter* m_pInterpreter;
                //std::string m_Error;
                boost::python::object m_Main;
                boost::python::object m_Global;
                std::vector<std::string> m_Paths;
                boost::signals2::signal<void(std::string)> m_onError;
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
        
        void set_error(std::string err);

    private:
        
        void* m_pUserData;
        static std::vector<Interpreter::Context*> s_Current;
        std::vector<std::string> m_Paths;
};

#endif

