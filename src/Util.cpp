#include "Util.h"
using namespace std;

glm::vec3 to_vec(std::shared_ptr<Meta> m)
{
    return glm::vec3(
        m->at<double>(0),
        m->at<double>(1),
        m->at<double>(2)
    );
}

std::shared_ptr<Meta> to_meta(glm::vec3 v)
{
    auto m = make_shared<Meta>();
    m->add<double>(v[0]);
    m->add<double>(v[1]);
    m->add<double>(v[2]);
    return m;
}

