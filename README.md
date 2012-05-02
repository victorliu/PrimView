PrimView
========

Overview
--------
PrimView is both a tool and an abstract specification for the visualization of geometric primitives for rapid geometric prototyping and debugging. The PrimView program itself is an 3D viewer which displays basic geometric primitives like points, lines, triangles, etc. based on a Lua scripting interface. The specification portion dictates what features a compatible viewer must implement, as well as a compact file format for geometric data exchange.

The file format is modeled after PostScript, providing basic commands to specify what should appear on screen, and the program is like a very basic GeomView. Note that the emphasis here is on geometry, not rendering. Thus, textures, lighting, meshes, and processing are beyond the scope of this specification, but not entirely ruled out for future extensions. Currently, color is the only basic non-geometric property that can be specified.

PrimView language specification
-------------------------------
The PrimView scripting language is an extension of Lua. There are only two symbols in the global namespace: `PVF` (to import the PrimView file format definitions) and `PrimView` (the table/namespace of the main PrimView functions.

### `PrimView.Point{x, y, z}`

> Adds a point to the current set of geometry, with cartesian coordinates (x, y, z). The point is assigned the next available index, starting from zero. Additional options may be specified after the z coordinate such as
>
>`PrimView.Point{x, y, z, color = {r, g, b}}`
>
> to specify the color of the point, where r, g, and b are floating point numbers in [0, 1].

### `PrimView.Line{i0, i1}`

> Adds a line segment to the current set of geometry from point index i0 to i1.
> Indices are zero-based and may be negative, indicating indexing from the end of the current list of points (-1 is the last point added, -2 is the next to last point, etc.).
> Additional options may also be specified (see `PrimView.Point`).

### `PrimView.Arrow{i0, i1}`

> Adds a arrow to the current set of geometry from point index i0 to i1.
> See `PrimView.Line` for indexing details.
> Additional options may also be specified (see `PrimView.Point`).

### `PrimView.Triangle{i0, i1, i2}`

> Adds a triangle to the current set of geometry with vertex point indices i0, i1, and i2. 
> See `PrimView.Line` for indexing details.
> Additional options may also be specified (see `PrimView.Point`).

### `PrimView.Quad{i0, i1, i2, i3}`

> Adds a quadrangle (not necessarily planar) to the current set of geometry with vertex point indices i0, i1, i2, and i3. 
> See `PrimView.Line` for indexing details.
> Additional options may also be specified (see `PrimView.Point`).


### `PrimView.Tetrahedron{i0, i1, i2, i3}`

> Adds a tetrahedron to the current set of geometry with vertex point indices i0, i1, i2, and i3. 
> See `PrimView.Line` for indexing details.
> Additional options may also be specified (see `PrimView.Point`).

### `PrimView.Sphere{ic, r}`

> Adds a sphere to the current set of geometry with center point index ic and radius r. 
> See `PrimView.Line` for indexing details.
> Additional options may also be specified (see `PrimView.Point`).

### `PrimView.Text{ic, str}`

> Adds a text label near the point index ic with string str.
> See `PrimView.Line` for indexing details.
> Additional options may also be specified (see `PrimView.Point`).

### `PrimView.SetOptions{name = value}`

> Sets various options related to how geometry should be displayed. Multiple options can be specified at once as a list.
> See the description of available options below.

### Options

#### Origin = {x, y, z}

> Sets the arcball rotation origin to the specified coordinates.

#### LineSize = r

> Sets the radius of cylinders used to denote lines to r.

#### LineShrink = f

> Lines can be displayed shorter than their actual length to prevent obscuring the endpoints.
> The factor f determines the fraction of the total line segment length by which the segment should be shrunk on each end.
> f = 0 denotes no shrinking.

#### TriangleShrink = f

> Triangles can be displayed smaller than their actual size to prevent obscuring the edges and vertices.
> The factor f determines the barycentric factor by which the vertices should be shrunk.
> f = 0 denotes no shrinking.

#### QuadShrink = f

> Quadrangles can be displayed smaller than their actual size to prevent obscuring the edges and vertices.
> The factor f determines the barycentric factor by which the vertices should be shrunk.
> f = 0 denotes no shrinking.

#### TetrahedronShrink = f

> Tetrahedra can be displayed smaller than their actual size to prevent obscuring the edges and vertices.
> The factor f determines the barycentric factor by which the vertices should be shrunk.
> f = 0 denotes no shrinking.

### PVF(v)

> Loads version v of the PrimView file format definitions into the global namespace.
> These definitions provide aliases for the PrimView geometry specification functions that are much shorter than the built-in names.
> The exact aliases depend on the version number specified.

PrimView file format
--------------------
A valid PrimView file must begin with the first `PVF(v)` where v currently must be 1. After this, the file can be any valid Lua script.

### Version 1 aliases

* `p` === `PrimView.Point`
* `l` === `PrimView.Line`
* `v` === `PrimView.Arrow`
* `t` === `PrimView.Triangle`
* `q` === `PrimView.Quad`
* `T` === `PrimView.Tetrahedron`
* `b` === `PrimView.Sphere`

PrimView viewer feature specification
-------------------------------------

The viewer must provide a basic 3D viewport with arcball rotation, display all the specified geometry, and provide basic viewing options.

### Geometry display

Points are rendered as spheres, lines as cylinders, and the rest should be obvious. Nonplanar quads are interpreted by the 3D pipeline accordingly.
Text should be displayed somewhere near where the specified point in screen space, but there are no strict requirements.
Rotation is to be performed by clicking and dragging a mouse button, and zooming by the scroll wheel.

Facilities must be provided to perform screen capture and to save and load the current view to disk.

Reference viewer
----------------
The reference viewer currently requires FreeGLUT and Lua 5.2.0 to compile.
