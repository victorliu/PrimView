-- This input script opens the following file and draws all the ATOMs
-- with the specified radius.

PDFfile = '1CRN.pdb'
atom_radius = 1

CPKcolor = {
	H = {1,1,1},
	C = {0,0,0},
	N = {0,0,1},
	O = {1,0,0},
	F = {0,0.5,0},
	Cl = {0,0.5,0},
	Br = {0.5,0,0},
	S = {1,1,0},
	P = {1,0.5,0},
	other = {1,0,1}
}

function trim(s)
	return string.match(s, '%a+')
end

for line in io.lines(PDFfile) do
	if string.sub(line, 1, 6) == 'ATOM  ' then
		x = string.sub(line, 31, 38)
		y = string.sub(line, 39, 46)
		z = string.sub(line, 47, 54)
		elsymb = string.sub(line, 77, 78)
		elsymb = trim(elsymb)
		color = CPKcolor[elsymb]
		if nil == color then
			color = CPKcolor['other']
		end
		PrimView.Point{x,y,z}
		PrimView.Sphere{-1,atom_radius, color=color}
	end
end