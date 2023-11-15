# Mesh Slicing GDExtension

## Compatibility

Godot 4.1

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
godot --editor --path ./demo
```

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
