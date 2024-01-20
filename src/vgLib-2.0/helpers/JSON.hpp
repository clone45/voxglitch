#pragma once

#include <jansson.h>
#include <stdbool.h>

namespace vgLib_v2
{

    class JSON
    {

    public:
        static double getNumber(json_t *root, const char *key)
        {
            json_t *json_value = json_object_get(root, key);
            if (json_value && json_is_number(json_value))
            {
                return json_number_value(json_value);
            }
            return 0.0; // or another default value if required
        }

        static int getInteger(json_t *root, const char *key)
        {
            json_t *json_value = json_object_get(root, key);
            if (json_value && json_is_integer(json_value))
            {
                return json_integer_value(json_value);
            }
            return 0; // or another default value if required
        }

        static bool getBoolean(json_t *root, const char *key)
        {
            json_t *json_value = json_object_get(root, key);
            if (json_value && json_is_boolean(json_value))
            {
                return json_boolean_value(json_value);
            }
            return false; // or another default value if required
        }

    };

} // namespace vgLib_v2