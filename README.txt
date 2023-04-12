                   cRTED by Hermit (Mihaly Horvath)
            -----------------------------------------------

 cRTED is a music player for my new Commodore Plus4 music format based on
the TMF format brought further by Siz. I made a music-collection based on
Luca's HVTC (High Voltage TED collection) found at plus4world.powweb.com
called HVTC-ted, including a tool to convert the .prg files to .ted.
The music included in this package doesn't contain all 877 tunes that can be
found in HVTC-ted because of the 2MB upload limit. If you want the complete
experience you can download the HVTC-ted collection to play it.

 cRTED has both Linux and Windows versions (just as my other tool cRSID has)
and the Linux version has a .deb installer in this package which takes care
of associating .ted, .tmf and .tel (playlist) music files to cRTED. (The only
dependence of the program is the SDL library, SDL.dll is included for Win,
and nothing else is essential from the program package, just the runnable.)
Windows version doesn't have an installer so you need to associate the types
manually if you want single-click playback. There's no file-open dialog in
cRTED, you can't open files from within the app. That said, there are still
many ways to open a list of files. Clicking on the '.tel' simple textfiles
open the files/folders listed in it. The folders given as argument or in
the playlist-file, are searched for music files recursively (including
subfolders). If there's no argument given to crted it will try to open the
music/playlist files in the current folder, or if none found, it will play
a nice multi-tune music by 4-mat and display a short help about the usage.
These possibilities can be combined so you can give more files/folders or
playlist-files as arguments.

There's a GUI by default, but commandline -mode can be selected by giving
'-cli' as an argument. Arguments can be added in any order btw but always the
one coming later has the priority in case of clash. (Windows can't execute an
app as both commandline and GUI app, so crted.exe is the GUI one crted.com is
the commandline one. The .com app can still open the GUI if it's ran without
the -cli argument, albeit with an accompanying console window. Only te .com
version can output text to the console, the GUI app outputs nothing there.

 The GUI must be straightforward, the buttons at the bottom restart/pause and
fast-forward the tune, or select subtune/tune, or mute/unmute the channels.
Keys are still useable in the GUI:
Enter:restart, SPACE:pause/continue, TAB:fast-forward, 1..9,+/-: select subtune,
cursor-up/down: adjust main volume, left/right: previous/next tune/subtune.
The Page-Up/Down can scroll the playlist fast (by 1 page each time).
The mouse-scrollwheel is usable to adjust the volume, in the playlist-area
(when there are more then 1 tunes in the list) you can scroll the playlist with
the wheels slowly or fast when moved over the scrollbar on the right. You can
even drag the scrollbar to navigate through the playlist very fast. Clicking on
a tune's title by the left button starts it. The scrollbar shows the display-
position with a whitish rectangle, the playback-position with a blackish line.
 There's a button to toggle Auto-advance in playlist. When you see 'rep' on it
it will enable 'repeat'/'loop' of tunes (that loop) and won't go to the next
tune/subtune when the playtime expires. When you see 'adv' on it, it will turn
the auto-advance mode back to normal.

 There's no config-file as such for the volume/stereo settings but the editable
crted.sh file in Linux or the crted.bat starter scripts can be used to start
crted with specific settings.

 The '-info' argument can display TED-info just like selecting a subtune
by giving it as a standalone number anywhere.
(These parameters can be given after the linux script 'crteda' too, for
 example in a folder full of TEDs play them by:  crteda - -info )
Buffersize can be set between 256..32768 by '-bufsize xx' argument.
The '-volume xx' argument can be given to set main-volume between 0..255.

The app-name now contains the 'R' because now a RealTED-like environment-mode
is supported more-or-less ('RealTEDmode' in source).
There are still some RTED tunes that are not played properly, mainly
the ones using ROM extensively, because due to the copyright restrictions
I didn't include a complete Plus4 ROM, just some snippets needed to play the
existing tunes. Only 1% of the tunes suffer (play too fast or a bit slow.)
I don't feel like debugging these tunes too now, enough is enough...

This release also contains shared and static library forms for better
inclusion in other TED-playback projects, maybe to port to RockBox later
like the cRSID which is now the main SID-playback engine in RockBox.

I completely eliminated global variables and definitions, except a
'cRTED_CPlus4' instance which is for faster access of struct members.
(Emulated CPlus4 memory accesses are made to this for faster operation.)

More info (the API for the library) is seen in the 'libcRTED.h' file.

To emulate the TED sound as best as possible I made measurements and
recording during te development of TEDzakker and took a lot of ideas/facts
from Plus4emu, FPGATED projects. I reused my audio-code completely from
the TEDzakker project. But this time it's open-sourced finally. The
distortion which I assume based on oscilloscope-measurements at the direct
output of the TED chip is disabled now (just as in the r3 version of
TEDzakker) until further confirmation of the phenomenon. Essentially the
volume must be a little bit more than twice of the individual channels'
volumes when both channels are on (due to the stronger driving of the
integrator on the output of the TED. But when I simply did this (seen
commented out in CPlus4/TED.h) the distortion became too evident, maybe
I'm not right about this at all...or not in the right way and need to
dwelve into this topic a bit more.

License is WTF: Do what the fuck you want with this code, but
                it would be nice mentioning me as the original author.

Have a good music-listening session...

                        Hermit Software Hungary (Mihaly Horvath) - March 2023
                              (Contact me by plus4world.powweb.co or CSDb PM)
