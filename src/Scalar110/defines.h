namespace scalar_110
{
  const int NUMBER_OF_STEPS = 16;
  const int NUMBER_OF_TRACKS = 12;
  const int NUMBER_OF_PARAMETERS = 8;
  const int NUMBER_OF_ENGINES = 3;
  const int NUMBER_OF_SAMPLES = 16;

  const int LCD_DISPLAY_X = 43;
  const float LCD_DISPLAY_Y = 9.4;
  const int LCD_DISPLAY_WIDTH = 160;
  const int LCD_DISPLAY_HEIGHT = 100;
  const float BAR_HORIZONTAL_PADDING = 1;

  const float PARAMETER_LABEL_WIDTH = 58;
  const float PARAMETER_LABEL_HEIGHT = 18;

  const unsigned int NUMBER_OF_ENGINE_DISPLAY_ROWS = 5;
  const unsigned int NUMBER_OF_SAMPLE_DISPLAY_ROWS = 5;

  // LCD related constants
  const unsigned int NUMBER_OF_LCD_PAGES = 3;

  // Array Indexes
  const unsigned int LCD_PAGE_ENGINE = 0;
  const unsigned int LCD_PAGE_PARAMETER_VALUES = 1;
  const unsigned int LCD_PAGE_SAMPLES = 2;

  const std::string ENGINE_NAMES[3] = {
    "ByteBeat",
    "8-bit Drums",
    "Sample Player"
  };

}
