Frobnicators 4k
===============
This is a tool for 4k demos

Uses crinkler (link.exe).

To generate shaders for release:

ruby packer.rb demo_folder

where demo_folder is the project folder in data

Music
------

4klang is the synth.

Workflow:
* OpenMPT
* Open the project or create new
** If new project: Load plugin, and add 4klang as an instrument plugin
* A "Alcatraz - 4klang Synth Plugin" should have shown up.
* Modifie instruments etc in the plugin, or load patch data
* Export the song from the 4klang plugin window

To expose a 4klang instrument in MPT, add a instrument, and set plugin to 4klang, and set MIDI channel to the right 4klang instrument