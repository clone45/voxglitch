#include <jansson.h>
#include <stdbool.h>

namespace vgLib_v2
{
    class SEQUENCERS
    {
    public:

        template <typename T>
        static json_t *serialize(const T &sequencers)
        {
            json_t *array = json_array();
            for (auto &sequencer : sequencers)
            {
                json_array_append_new(array, sequencer.serialize());
            }
            return array;
        }

        template <typename T>
        static void deserialize(T &sequencers, json_t *array)
        {
            if (array && json_is_array(array))
            {
                size_t index = 0;
                for (auto &sequencer : sequencers)
                {
                    json_t *json_sequencer = json_array_get(array, index++);
                    sequencer.deserialize(json_sequencer);
                }
            }
        }
    };
} // namespace vgLib_v2

    