# === I generated this code with an AI ===

bl_info = {
    "name": "3D Engine Integration",
    "blender": (2, 80, 0),
    "category": "Object",
}

import bpy

# Array of class property dictionaries with inheritance
class_properties = [
    {
        "0_class": "DrawNode",
        "properties": [
            ("material", "default")
        ]
    },
    {
        "0_class": "Player",
        "properties": []
    },
    {
        "0_class": "Sun",
        "properties": []
    },
    {
        "0_class": "PhysCube",
        "properties": []
    },
    {
        "0_class": "DynSphere",
        "properties": []
    },
    {
        "0_class": "GoalRing",
        "inherits": "DrawNode",
        "properties": []
    }
]

# Function to get inherited properties
def get_inherited_properties(class_name):
    properties = []
    for class_property in class_properties:
        if class_property["0_class"] == class_name:
            if "inherits" in class_property:
                properties += get_inherited_properties(class_property["inherits"])
            properties += class_property["properties"]
    return properties

# Function to find common ancestor
def find_common_ancestor(class1, class2):
    ancestry1 = set()
    ancestry2 = set()
    
    current_class = class1
    while current_class:
        ancestry1.add(current_class)
        current_class = next((cp.get("inherits") for cp in class_properties if cp["0_class"] == current_class), None)
    
    current_class = class2
    while current_class:
        ancestry2.add(current_class)
        current_class = next((cp.get("inherits") for cp in class_properties if cp["0_class"] == current_class), None)
    
    common_ancestors = ancestry1.intersection(ancestry2)
    return common_ancestors.pop() if common_ancestors else None

# Define the operator to set properties
class OBJECT_OT_set_class_properties(bpy.types.Operator):
    bl_idname = "object.set_class_properties"
    bl_label = "Set Class Properties"
    
    class_name: bpy.props.StringProperty()

    def execute(self, context):
        for obj in context.selected_objects:
            old_class = obj.get("0_class")
            if not old_class:
                # No old class, add new properties directly
                properties = get_inherited_properties(self.class_name)
                obj["0_class"] = self.class_name
                for name, value in properties:
                    obj[name] = value
            else:
                # Remove properties that don't match
                common_ancestor = find_common_ancestor(old_class, self.class_name)
                while old_class != common_ancestor:
                    old_properties = get_inherited_properties(old_class)
                    for name, value in old_properties:
                        if name in obj:
                            del obj[name]
                    old_class = next((cp.get("inherits") for cp in class_properties if cp["0_class"] == old_class), None)
                
                # Add properties from common ancestor to new class
                new_properties = get_inherited_properties(self.class_name)
                obj["0_class"] = self.class_name
                for name, value in new_properties:
                    obj[name] = value

        self.report({'INFO'}, f"Set properties for class '{self.class_name}'")
        return {'FINISHED'}

# Define the operator to remove all properties
class OBJECT_OT_remove_all_properties(bpy.types.Operator):
    bl_idname = "object.remove_all_properties"
    bl_label = "Remove All Properties"

    def execute(self, context):
        for obj in context.selected_objects:
            obj_keys = list(obj.keys())
            for key in obj_keys:
                del obj[key]
        
        self.report({'INFO'}, "Removed all properties")
        return {'FINISHED'}

# Define the panel
class OBJECT_PT_class_panel(bpy.types.Panel):
    bl_label = "3D Engine"
    bl_idname = "OBJECT_PT_class_panel"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"
    
    def draw(self, context):
        layout = self.layout
        layout.label(text="General")
        
        # Dropdown for class selection
        row = layout.row()
        row.prop(context.scene, "selected_class", text="Class")

        # Button to set properties
        row = layout.row()
        op = row.operator("object.set_class_properties", text="Set Class Properties")
        op.class_name = context.scene.selected_class

        # Button to remove all properties
        row = layout.row()
        row.operator("object.remove_all_properties", text="Remove All Properties")

# Register the classes
def register():
    bpy.utils.register_class(OBJECT_OT_set_class_properties)
    bpy.utils.register_class(OBJECT_OT_remove_all_properties)
    bpy.utils.register_class(OBJECT_PT_class_panel)
    bpy.types.Scene.selected_class = bpy.props.EnumProperty(
        items=[(class_property["0_class"], class_property["0_class"], "") for class_property in class_properties],
        name="Class"
    )

# Unregister the classes
def unregister():
    bpy.utils.unregister_class(OBJECT_OT_set_class_properties)
    bpy.utils.unregister_class(OBJECT_OT_remove_all_properties)
    bpy.utils.unregister_class(OBJECT_PT_class_panel)
    del bpy.types.Scene.selected_class

if __name__ == "__main__":
    register()
