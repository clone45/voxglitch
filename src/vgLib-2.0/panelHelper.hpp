#pragma once

#include <functional>
#include <string>
#include <regex>

class PanelHelper {

public:

    PanelHelper(rack::ModuleWidget* moduleWidget);
    void loadPanel(const std::string& filename);
    void loadPanel(const std::string& filename1, const std::string& filename2);
    
    struct NamedPosition {
        std::string name;
        Vec position;
    };

    Vec findNamed(const std::string& name);
    std::vector<NamedPosition> findPrefixed(const std::string& prefix);
    std::vector<NamedPosition> findMatched(const std::string& pattern);

    void forEachPrefixed(const std::string& prefix, const std::function<void(unsigned int, const Vec&)>& callback);
    void forEachMatched(const std::string& regex, const std::function<void(const std::vector<std::string>&, const Vec&)>& callback);
    
private:
    rack::ModuleWidget* m_moduleWidget;
    std::shared_ptr<rack::Svg> m_svg;

    void forEachShape(const std::function<void(NSVGshape*)>& callback);
    Vec getBoundsCenter(float* bounds);

};

// Implementation

PanelHelper::PanelHelper(rack::ModuleWidget* moduleWidget)
    : m_moduleWidget(moduleWidget), m_svg(nullptr) {}


void PanelHelper::forEachShape(const std::function<void(NSVGshape*)>& callback) {
    if (!m_svg || !m_svg->handle) return;

    for (NSVGshape* shape = m_svg->handle->shapes; shape != nullptr; shape = shape->next) {
        callback(shape);
    }
}

Vec PanelHelper::getBoundsCenter(float* bounds) {
    return Vec((bounds[0] + bounds[2]) / 2, (bounds[1] + bounds[3]) / 2);
}

void PanelHelper::loadPanel(const std::string& filename) {
    if (!m_svg) {
        auto panel = rack::createPanel(filename);
        m_svg = panel->svg;
        m_moduleWidget->setPanel(panel);
    }
}

void PanelHelper::loadPanel(const std::string& filename1, const std::string& filename2) {
    if (!m_svg) {
        ThemedSvgPanel *panel = rack::createPanel(filename1, filename2);
        m_svg = panel->lightSvg;
        m_moduleWidget->setPanel(panel);
    }
}

Vec PanelHelper::findNamed(const std::string& name) {
    Vec result;
    forEachShape([&](NSVGshape* shape) {
        if (std::string(shape->id) == name) {
            result = getBoundsCenter(shape->bounds);
        }
    });
    return result;
}

std::vector<PanelHelper::NamedPosition> PanelHelper::findPrefixed(const std::string& prefix) {
    std::vector<NamedPosition> result;
    forEachShape([&](NSVGshape* shape) {
        std::string id(shape->id);
        if (id.compare(0, prefix.length(), prefix) == 0) {
            result.push_back({id, getBoundsCenter(shape->bounds)});
        }
    });
    return result;
}

std::vector<PanelHelper::NamedPosition> PanelHelper::findMatched(const std::string& pattern) {
    std::vector<NamedPosition> result;
    std::regex regex(pattern);
    forEachShape([&](NSVGshape* shape) {
        std::string id(shape->id);
        std::smatch match;
        if (std::regex_search(id, match, regex)) {
            std::vector<std::string> captures;
            for (size_t i = 1; i < match.size(); ++i) {
                captures.push_back(match[i].str());
            }
            result.push_back({id, getBoundsCenter(shape->bounds)});
        }
    });
    return result;
}

void PanelHelper::forEachPrefixed(const std::string& prefix, const std::function<void(unsigned int, const Vec&)>& callback) {
    auto positions = findPrefixed(prefix);
    for (unsigned int i = 0; i < positions.size(); ++i) {
        callback(i, positions[i].position);
    }
}

void PanelHelper::forEachMatched(const std::string& regex, const std::function<void(const std::vector<std::string>&, const Vec&)>& callback) {
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
