import inkex

class CreatePanelBackground(inkex.EffectExtension):
    def add_arguments(self, pars):
        pars.add_argument("--tab", type=str, default="options")
        pars.add_argument("--mode", type=str, default="light", help="Choose light or dark mode")

    def effect(self):
        # Convert document dimensions from px to document units
        doc_width = self.svg.unittouu(f"{self.svg.viewport_width}px")
        doc_height = self.svg.unittouu(f"{self.svg.viewport_height}px")

        # Set fill color based on mode
        fill_color = "#e7e7e7" if self.options.mode == "light" else "#18181a"

        # Create a new rectangle for the panel background
        rect = inkex.Rectangle(x="0", y="0", width=str(doc_width), height=str(doc_height))

        # Style the rectangle
        rect.style = {
            'fill': fill_color,
            'fill-opacity': '1',
            'opacity': '1',
            'stroke': 'none'  # No stroke for a clean edge
        }

        # Add the new rectangle to the current layer
        current_layer = self.svg.get_current_layer()
        current_layer.add(rect)

        # Move the rectangle to the bottom
        rect.set("inkscape:groupmode", "layer")
        self.svg.add(rect)
        
        # Ensure it's at the bottom by moving it to the start of the SVG's child list
        self.svg.remove(rect)
        self.svg.insert(0, rect)

if __name__ == '__main__':
    CreatePanelBackground().run()