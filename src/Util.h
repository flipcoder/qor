#ifndef UTIL_H_0RIT3UWJ
#define UTIL_H_0RIT3UWJ

#include <glm/glm.hpp>
#include "kit/meta/meta.h"

glm::vec3 to_vec(std::shared_ptr<Meta> m);
std::shared_ptr<Meta> to_meta(glm::vec3 v);

#endif

