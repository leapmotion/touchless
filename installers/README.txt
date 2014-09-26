To generate new package for windows

1) kick ocuserv3 windows platform release build (public installer)

2) from build products archive dropped into \\ocuserv2\builds\BuildProducts\Public\win\release 
   extract Touchless.exe 
   into Touchless For Windows\ in this folder

3) from \\ocuserv2\common\TouchlessDocs\win 
   copy static-docs\
   into Touchless For Windows\

4) Edit installer.nsi and increment VersionNumber each time you generate a new installer.

5) Build installer.nsi

6) drop Touchless_LM.exe (installer output) into .zip

7) rename .zip to increment build number (4th dot)

8) upload to warehouse https://warehouse.leapmotion.com/organizations/leap-motion--2/apps/touchless-for-windows,
update app details and binaries - when uploading the binary be sure to select 'I would like to use Leap Motion Code Signing'

9) fast forward the app approval process https://warehouse.leapmotion.com/admin/certifications

10) activate the new version https://warehouse.leapmotion.com/organizations/leap-motion--2/apps/touchless-for-windows


To generate new package for mac

1) If there are any changes to docs they need to be updated at \\ocuserv2\common\TouchlessDocs\mac

2) kick mac-build2 platform release build (public installer)

3) from \\ocuserv2\builds\BuildProducts\Public\mac\release copy most recent version of TouchlessForMac_LM_X.X.X.XXXX_OSX.dmg
into this folder for archival purposes.

4) upload to warehouse https://warehouse.leapmotion.com/organizations/leap-motion--2/apps/touchless-for-mac

5) fast forward the app approval process https://warehouse.leapmotion.com/admin/certifications

6) activate the new version https://warehouse.leapmotion.com/organizations/leap-motion--2/apps/touchless-for-mac

NOTES
until touchless builds are decoupled from platform builds you should never publish a version of touchless linked to a
newer minot version of leap internal the current published version of the software or it will not connect to leapd/leapservice.
e.g. if the current published version of leap core services is 1.0 and touchless is linked against a leap internal
on a branch with the version number set to 1.1 or higher it will be unable to connect to the 1.0 service/daemon.
a 1.0.4 client will still connect to a 1.0.0 service - the version check is only against major and minor numbers.
patch number is ignored.


imageformats only needed in Touchless For Windows/ if using Qt HTML viewing window to show docs.
Windows is showing in user's default browser.
