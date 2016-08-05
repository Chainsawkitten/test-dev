#pragma once

#include "ComponentEditor.hpp"

class Entity;
class Font;

namespace GUI {
    class FloatEditor;
    
    /// Used to edit a lens component.
    class LensEditor : public ComponentEditor {
        public:
            /// Create new transform editor.
            /**
             * @param parent Parent widget.
             */
            LensEditor(Widget* parent);
            
            /// Destructor.
            ~LensEditor() override;
            
            /// Set the entity to edit the transform component of.
            /**
             * @param entity %Entity to edit.
             */
            void SetEntity(Entity* entity) override;
            
        private:
            Font* font;
            
            FloatEditor* fieldOfViewEditor;
            FloatEditor* zNearEditor;
            FloatEditor* zFarEditor;
    };
}