#include "defines.h"

struct Vulcan : Module
{
  bool grid_data[COLS][ROWS];

  enum ParamIds
  {
    NUM_PARAMS
  };
  enum InputIds
  {
    NUM_INPUTS
  };
  enum OutputIds
  {
    NUM_OUTPUTS
  };
  enum LightIds
  {
    NUM_LIGHTS
  };

  Vulcan()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    for (unsigned int row = 0; row < ROWS; row++)
    {
      for (unsigned int col = 0; col < COLS; col++)
      {
        grid_data[col][row] = false;
      }
    }
  }

  void onRandomize() override
  {
    auto gen = std::bind(std::uniform_int_distribution<>(0, 1), std::default_random_engine());

    for (unsigned int row = 0; row < ROWS; row++)
    {
      for (unsigned int col = 0; col < COLS; col++)
      {
        grid_data[col][row] = (bool) gen();
      }
    }
  }

  json_t *dataToJson() override
  {
    json_t *json_root = json_object();

    // Do save stuff here

    return json_root;
  }

  void dataFromJson(json_t *json_root) override
  {
    // Do load stuff here
  }

  void process(const ProcessArgs &args) override
  {
  }
};

struct GridWidget : TransparentWidget
{
  Vulcan *module;

  GridWidget()
  {
    box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    // Debugging code for draw area, which often has to be set experimentally
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(vg, nvgRGBA(120, 120, 120, 100));
    nvgFill(vg);

    if (module)
    {
      for (unsigned int column = 0; column < COLS; column++)
      {
        for (unsigned int row = 0; row < ROWS; row++)
        {
          float x = (column * CELL_WIDTH) + (column * CELL_PADDING);
          float y = ((ROWS - row - 1) * CELL_HEIGHT) + ((ROWS - row - 1) * CELL_PADDING);

          // Draw square
          nvgBeginPath(vg);
          nvgRect(vg, x, y, CELL_WIDTH, CELL_HEIGHT);

          if (module->grid_data[column][row] == true)
          {
            nvgFillColor(vg, nvgRGB(50, 50, 50));
          }
          else
          {
            nvgFillColor(vg, nvgRGB(160, 160, 160));
          }

          nvgFill(vg);
        }
      }
    }

    nvgRestore(vg);
  }

  std::pair<unsigned int, unsigned int> getRowAndColumnFromVec(Vec position)
  {
    unsigned int row = ROWS - (position.y / (CELL_HEIGHT + CELL_PADDING));
    unsigned int column = position.x / (CELL_WIDTH + CELL_PADDING);

    row = clamp(row, 0, ROWS - 1);
    column = clamp(column, 0, COLS - 1);

    return {row, column};
  }

  void onButton(const event::Button &e) override
  {
    if (isMouseInDrawArea(e.pos))
    {
      if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        e.consume(this);

        unsigned int row, column;
        std::tie(row, column) = getRowAndColumnFromVec(e.pos);

        module->grid_data[column][row] = !module->grid_data[column][row];
      }
    }
  }

  void onEnter(const event::Enter &e) override
  {
    TransparentWidget::onEnter(e);
  }

  void onLeave(const event::Leave &e) override
  {
    TransparentWidget::onLeave(e);
  }

  void onHover(const event::Hover &e) override
  {
    TransparentWidget::onHover(e);
  }

  bool isMouseInDrawArea(Vec position)
  {
    if (position.x < 0) return (false);
    if (position.y < 0) return (false);
    if (position.x >= DRAW_AREA_WIDTH) return (false);
    if (position.y >= DRAW_AREA_HEIGHT) return (false);
    return (true);
  }

  void step() override
  {
    TransparentWidget::step();
  }
};

struct VulcanWidget : ModuleWidget
{
  VulcanWidget(Vulcan *module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/vulcan_front_panel.svg")));

    GridWidget *grid_widget = new GridWidget();
    grid_widget->box.pos = Vec(120, 20);
    grid_widget->module = module;
    addChild(grid_widget);
  }
};