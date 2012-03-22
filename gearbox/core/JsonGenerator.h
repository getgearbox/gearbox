#ifndef GEARBOX_JSON_GENERATOR_H
#define GEARBOX_JSON_GENERATOR_H

#include <gearbox/core/Json.h>
#include <yajl/yajl_gen.h>
#include "JsonPrivate.h"

namespace Gearbox {

class Json::Generator {
 public:
  static std::string generate(const Json & json, bool beautify = false);

 private:
  static void generate(yajl_gen g, const Json * j);
  static void generateObject(yajl_gen g, const Json * j);
  static void generateArray(yajl_gen g, const Json * j);
  
};

} // namespace

#endif // GEARBOX_JSON_GENERATOR_H
