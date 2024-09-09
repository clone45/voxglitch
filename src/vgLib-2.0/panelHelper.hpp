// Modified from https://community.vcvrack.com/t/death-to-helper-py-long-live-svghelper/19427
// Assumes that panel control placement is on the light (default) SVG.

#pragma once

#include <functional>
#include <string>
#include <regex>
#include <rack.hpp>

struct PanelHelper
{
    rack::ModuleWidget* m_moduleWidget;
    std::shared_ptr<rack::Svg> m_svg;

    struct NamedPosition {
        std::string name;
        Vec position;
    };

    PanelHelper(rack::ModuleWidget* moduleWidget)
        : m_moduleWidget(moduleWidget), m_svg(nullptr) {}

    // Loads the panel from the given filename
    // Use this if you are using only the light version of the panel
    
    void loadPanel(const std::string& filename)
    {
        if (!m_svg) {
            auto panel = rack::createPanel(filename);
            m_svg = panel->svg;
            m_moduleWidget->setPanel(panel);
        }
    }

    // Loads the panel from the given filenames
    // Use this if you are using both the light and dark versions of the panel

    void loadPanel(const std::string& filename1, const std::string& filename2)
    {
        if (!m_svg) {
            ThemedSvgPanel *panel = rack::createPanel(filename1, filename2);
            m_svg = panel->lightSvg;
            m_moduleWidget->setPanel(panel);
        }
    }

    // Finds the position of a named control
    // Returns the center of the control's bounding box
    // If the control isn't found, returns Vec(0, 0)

    Vec findNamed(const std::string& name)
    {
        Vec result;
        forEachShape([&](NSVGshape* shape) {
            if (std::string(shape->id) == name) {
                result = getBoundsCenter(shape->bounds);
            }
        });
        return result;
    }

    std::vector<NamedPosition> findPrefixed(const std::string& prefix)
    {
        std::vector<NamedPosition> result;
        forEachShape([&](NSVGshape* shape) {
            std::string id(shape->id);
            if (id.compare(0, prefix.length(), prefix) == 0) {
                result.push_back({id, getBoundsCenter(shape->bounds)});
            }
        });
        return result;
    }

    std::vector<NamedPosition> findMatched(const std::string& pattern)
    {
        std::vector<NamedPosition> result;
        std::regex regex(pattern);
        forEachShape([&](NSVGshape* shape) {
            std::string id(shape->id);
            std::smatch match;
            if (std::regex_search(id, match, regex)) {
                result.push_back({id, getBoundsCenter(shape->bounds)});
            }
        });
        return result;
    }

    void forEachPrefixed(const std::string& prefix, const std::function<void(unsigned int, const Vec&)>& callback)
    {
        auto positions = findPrefixed(prefix);
        for (unsigned int i = 0; i < positions.size(); ++i) {
            callback(i, positions[i].position);
        }
    }

    void forEachMatched(const std::string& regex, const std::function<void(const std::vector<std::string>&, const Vec&)>& callback)
    {
        std::regex re(regex);
        forEachShape([&](NSVGshape* shape) {
            std::string id(shape->id);
            std::smatch match;
            if (std::regex_search(id, match, re)) {
                std::vector<std::string> captures;
                for (size_t i = 1; i < match.size(); ++i) {
                    captures.push_back(match[i].str());
                }
                callback(captures, getBoundsCenter(shape->bounds));
            }
        });
    }

private:
    void forEachShape(const std::function<void(NSVGshape*)>& callback)
    {
        if (!m_svg || !m_svg->handle) return;

        for (NSVGshape* shape = m_svg->handle->shapes; shape != nullptr; shape = shape->next) {
            callback(shape);
        }
    }

    Vec getBoundsCenter(float* bounds)
    {
        return Vec((bounds[0] + bounds[2]) / 2, (bounds[1] + bounds[3]) / 2);
    }
};
