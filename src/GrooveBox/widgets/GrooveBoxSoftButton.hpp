struct GrooveBoxSoftButton : LEDButton {
    GrooveBoxSoftButton() {
        this->setSvg(Svg::load(asset::system("res/groovebox/groove_box_soft_button.svg")));
    }
};
