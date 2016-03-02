#include <boost/python.hpp>
#include <Python.h>
#include "Interpreter.h"
#include "PythonBindings.h"
using namespace std;

std::vector<Interpreter :: Context*> Interpreter :: s_Current;

namespace py = boost::python;

Interpreter::Interpreter(
    const char* name,
    void* userdata,
    std::vector<std::string> paths
):
    m_pUserData(userdata),
    m_Paths(paths)
{
    if(s_Current.capacity() < 4)
        s_Current.reserve(4);
    if(!Py_IsInitialized()){
        PyImport_AppendInittab(name, Scripting::initqor);
        Py_Initialize();
    }
    // Py_GetVersion()
}

Interpreter::~Interpreter()
{
    //if(Py_IsInitialized())
    //    Py_Finalize();
}

Interpreter::Context :: Context(Interpreter* interp):
    m_pInterpreter(interp)
{
    clear();
}

Interpreter::Context :: ~Context() {
}

void Interpreter::Context :: clear()
{
    m_Main = py::import("__main__");
    m_Global = m_Main.attr("__dict__");
    m_Paths = m_pInterpreter->paths();
}

bool Interpreter::Context :: execute_file(const std::string& fn)
{
    Interpreter::Selection s(this);
    try{
        py::exec_file(fn.c_str(), m_Global, m_Global);
    }catch(const std::invalid_argument& e) {
        PyErr_Print();
        error("no such file");
        return false;
    }catch(const py::error_already_set& e) {
        PyErr_Print();
        error("python error");
        return false;
    }
    return true;
}

bool Interpreter::Context :: execute_string(const std::string& code)
{
    Interpreter::Selection s(this);
    try{
        py::exec(code.c_str(), m_Global, m_Global);
    }catch(const py::error_already_set& e) {
        PyErr_Print();
        error("python error");
        return false;
    }
    return true;
}

py::object Interpreter::Context :: evaluate_string(const std::string& code)
{
    Interpreter::Selection s(this);
    try{
        return py::eval(code.c_str(), m_Global, m_Global);
    }catch(const py::error_already_set& e) {
        PyErr_Print();
        error("python error");
        return py::object();
    }
}

//void Interpreter :: set_error(std::string err)
//{
//    m_onError(err);
//}

