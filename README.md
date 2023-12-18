# Mesh Slicing GDExtension

This GDExtension implements Mesh Slicing. For usage see the demo project.

<img src="mesh-slicing-demo.gif" alt="Mesh Slicing Animation" width="440"/>

## Limitations

- Slicing non-convex meshes will produce unexpected results.
- Supported Mesh Types are: `ArrayMesh`, `PrimitiveMesh`
- Only Triangle Meshes are supported (`PRIMITIVE_TRIANGLES`)
- Only UVs and Normals are transferred from the original mesh.
- The created surface does not have UVs.

## Compatibility

Godot 4.2

## Cloning

```sh
git clone --recurse-submodules link-to-this-repo
```

If you already cloned and forgot `--recurse-submodules` :

```sh
git submodule update --init --recursive
```

## Building

> See also [GDExtension Example from Godot Docs](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html)

Requirements: C++ Compiler, SCons as a build tool

```sh
scons
```
## Run Demo Project

```sh
godot --path ./demo --editor
```

## Use it in your project

After building, copy the contents of `demo/bin/` into your project.

## Editing with Visual Studio Code

Add the `includePath` from the following code snippet to your `.vscode/c_cpp_properties.json`

```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/godot-cpp/gen/include",
                "${workspaceFolder}/src"
            ],
// ...
```
