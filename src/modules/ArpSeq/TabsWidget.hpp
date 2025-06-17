struct Tab : TransparentWidget
{
    std::string label;
    float x, y, w, h;
    NVGcolor color_default;
    NVGcolor color_hover;
    NVGcolor color_selected;
    NVGcolor label_color;
    NVGcolor label_color_selected;
    NVGcolor label_color_deselected;
    bool is_hovered = false;
    bool is_selected = false;

    Tab(std::string lbl, float x, float y, float w, float h,
        NVGcolor col_default, NVGcolor color_hover, NVGcolor col_selected, NVGcolor label_color_selected, NVGcolor label_color_deselected) 
    {
        this->label = lbl;
        this->x = x; // Assign position and dimensions directly.
        this->y = y;
        this->w = w;
        this->h = h;
        this->color_default = col_default;
        this->color_hover = color_hover;
        this->color_selected = col_selected;
        this->label_color_selected = label_color_selected;
        this->label_color_deselected = label_color_deselected;
        this->label_color = label_color_deselected;

        this->box.pos = Vec(x, y);
        this->box.size = Vec(w, h);
    }


    // Draw the tab
    void draw(NVGcontext *vg) override
    {
        NVGcolor tab_color = is_hovered ? color_hover : color_default;
        if(isSelected()) tab_color = color_selected;

        nvgSave(vg);

        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, w, h);
        nvgFillColor(vg, tab_color);
        nvgFill(vg);

        // Draw the tab label
        nvgFontSize(vg, 12);
        nvgTextLetterSpacing(vg, 0);
        nvgFillColor(vg, label_color);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(vg, w/2, h/2, label.c_str(), NULL);
    
        nvgRestore(vg);
    }

    // Check if the mouse is over the tab
    bool isMouseOver(Vec mouse_pos) 
    {
        return mouse_pos.x >= x && mouse_pos.x <= x + w &&
               mouse_pos.y >= y && mouse_pos.y <= y + h;
    }

    void onHover(const event::Hover &e) override {
        TransparentWidget::onHover(e); // Call the parent class's onHover
        is_hovered = true;
        e.consume(this);
    }

    void onLeave(const event::Leave &e) override {
        TransparentWidget::onLeave(e); // Call the parent class's onLeave
        is_hovered = false;
        e.consume(this);
    }

    void onLeave() 
    {
        is_hovered = false;
    }

    bool isSelected() 
    {
        return is_selected;
    }

    void setSelected(bool selected) 
    {
        is_selected = selected;
        (is_selected) ? label_color = label_color_selected : label_color = label_color_deselected;
    }

};

struct TabsWidget : TransparentWidget
{
    unsigned int selected_tab = 0;
    bool hover = false;

    unsigned int *module_tab_ptr = NULL;

    float TAB_WIDTH = 90.016 + .1; // .1 is to cover the seam
    float TAB_HEIGHT = 74.6095;
    float TAB_VERTICAL_PADDING = 1.0;

    std::vector<std::vector<Widget *>> tab_contents;
    std::vector<Tab*> tabs;  // Vector of Tab objects

    NVGcolor tab_color_default = nvgRGBA(30, 24, 2, 255);
    NVGcolor tab_color_hover = nvgRGBA(38, 31, 3, 255);
    NVGcolor tab_color_selected = nvgRGBA(51, 42, 4, 255);
    NVGcolor label_color_deselected = nvgRGBA(255, 215, 20, 130);
    NVGcolor label_color_selected = nvgRGBA(255, 215, 20, 255);

    // std::vector<std::string> tab_labels = {};

    TabsWidget(unsigned int *tab_ptr)
    {
        module_tab_ptr = tab_ptr;
    }

    void addTab(std::string label, std::vector<Widget *> widgets, bool is_selected = false) 
    {
        // Create a new Tab object
        float number_of_tabs = (float) tabs.size();
        float y = number_of_tabs * (TAB_HEIGHT + TAB_VERTICAL_PADDING);

        // Adjust this widget's box to fit all of the tabs
        this->box.size.y = (TAB_HEIGHT + TAB_VERTICAL_PADDING) * (number_of_tabs + 1);
        this->box.size.x = TAB_WIDTH;

        Tab *new_tab = new Tab(
            label, 
            0.0, 
            y, 
            TAB_WIDTH, 
            TAB_HEIGHT, 
            tab_color_default, 
            tab_color_hover, 
            tab_color_selected,
            label_color_selected, 
            label_color_deselected
        );

        new_tab->setSelected(is_selected);

        // Add the Tab object to the tabs vector
        tabs.push_back(new_tab);

        // Add the widgets for the new tab to the tab_contents vector
        tab_contents.push_back(widgets);

        // Hide all of the widgets in the tab, unless it's the first tab
        if (tabs.size() > 1) {
            hideTab(tabs.size() - 1);
        }

        this->addChild(new_tab);

    }

    void onButton(const event::Button &e) override 
    {
        // Check if the button event is within the rect of the entire TabsWidget
        if (e.pos.x > TAB_WIDTH || e.pos.y > (TAB_HEIGHT + TAB_VERTICAL_PADDING) * tabs.size()) 
        {
            return;
        }

        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) 
        {
            e.consume(this);

            // Iterate through the tabs to find which one was clicked
            for (unsigned int i = 0; i < tabs.size(); ++i) 
            {
                if (tabs[i]->isMouseOver(e.pos)) 
                {
                    // If a tab is clicked, switch to that tab
                    this->switchTabs(selected_tab, i);
                    break;
                }
            }
        }
    }

    void switchTabs(unsigned int previous_selected_tab, unsigned int new_selected_tab) 
    {
        // Hide the widgets associated with the previously selected tab
        if (previous_selected_tab < tabs.size()) {
            hideTab(previous_selected_tab);
        }

        // Show the widgets associated with the new selected tab
        if (new_selected_tab < tabs.size()) {
            showTab(new_selected_tab);
        }

        // Update the selected_tab variable
        selected_tab = new_selected_tab;

        // Update the module state, if applicable
        if (module_tab_ptr != NULL) {
            *module_tab_ptr = new_selected_tab;
        }

        // Call the setSelected method for each tab
        for (unsigned int i = 0; i < tabs.size(); ++i) {
            tabs[i]->setSelected(i == new_selected_tab);
            tabs[i]->label_color = i == new_selected_tab ? label_color_selected : label_color_deselected;
        }
    }


    void hideTab(unsigned int tab) {
        if (tab < tabs.size()) {
            for (Widget* widget : tab_contents[tab]) {
                widget->visible = false;
            }
        }
    }

    void showTab(unsigned int tab) {
        if (tab < tabs.size()) {
            for (Widget* widget : tab_contents[tab]) {
                widget->visible = true;
            }
        }
    }

    void onHover(const event::Hover &e) override {
        TransparentWidget::onHover(e); // Call the parent class's onHover
        for (Tab* tab : tabs) {
            if (tab->box.contains(e.pos)) {
                tab->onHover(e);
            }
            else {
                tab->onLeave();
            }
        }
        e.consume(this);
    }

    void onLeave(const event::Leave &e) override {
        TransparentWidget::onLeave(e); // Call the parent class's onLeave
        for (Tab* tab : tabs) {
            tab->onLeave(e); // Call onLeave for each tab
        }
        e.consume(this);
    }


};