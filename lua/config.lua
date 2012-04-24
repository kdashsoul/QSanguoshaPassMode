
-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
	version = "20120405",
	version_name = "踏青版",
	mod_name = "official",
	kingdoms = { "wei", "shu", "wu", "qun", "god"},
	package_names = {
	"StandardCard",
        "StandardExCard",
        "Maneuvering",
        "SPCard",
        "Nostalgia",

        "Standard",
        "Wind",
        "Fire",
        "Thicket",
        "Mountain",
        "God",
        "SP",
        "YJCM",
        "YJCM2012",
        "Special3v3",
        "BGM",
	"Pass"
	},

	scene_names = {
        "Custom"
	},
}
for i=1, 7 do
	local scene_name = ("PassMode_%02d"):format(i)
	table.insert(config.scene_names, scene_name)
end

for i=1, 21 do
	local scene_name = ("MiniScene_%02d"):format(i)
	table.insert(config.scene_names, scene_name)
end

