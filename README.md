# FlingXBMC

FlintXBMC is base on XBMC(Now named Kodi). It integrates the [Flint SDK](http://github.com/openflint/flint-android-sender-sdk), so you can enjoy videos on your TV by any device which has Flint protocol inside, such as [MatchStick](www.matchstick.tv).

* Right now, only Android is supported.

## How to build

1. Download 'master'.
2. Build libxbmc.so (for reference, see [docs/README.android](https://github.com/wudongyang/flintxbmc/blob/master/docs/README.android).).
3. Run flintconfig.sh.
4. Import the FlingXBMC project for Eclipse.
5. Build and export xbmc.apk by Eclipse.


## How to fling videos

### Requirements
* A device which has the Flint protocol inside, such as [MatchStick](http://matchstick.tv).
* A TV set or monitor with an HDMI port and your MatchStick device plugged into your TV.
* [Download](http://www.matchstick.tv/setup) and install the MatchStick Setup App on your smart phone (iOS or Android).

### Steps

1. Setup the MatchStick device connected to internet via WiFi by MatchStick Setup App.
2. Run FlintXBMC and make sure that FlintXBMC is connected the same WiFi as MatchStick device.
3. Tap the fling icon"![fling icon](https://raw.githubusercontent.com/wudongyang/flintxbmc/master/addons/skin.confluence/media/Matchstick_disconnectFO.png)" on the FlintXBMC screen. If you can't find it, see the FAQ.
4. Select a MatchStick device.
5. Play a video; it should now be playing on your TV.


## Links
* [MatchStick](http://www.matchstick.tv)
* [OpenFlint](http://openflint.org/)

## FAQ
### Why do we need Eclipse.
- Some library resources are required as dependencies for FlintXBMC. They need to be imported as library projects by Eclipse.
- We can debug the Java codes.

### Build Error when using Eclipse.
- Go to Window -> Preferences -> Android -> Lint Error Checking，uncheck “Run full error check when exporting app and abort if fatal errors are found”.

### Can't find Fling icon.
- Make sure that FlintXBMC is connected the same WiFi as your MatchStick device.
- Try to restarting FlintXBMC or rebooting your MatchStick devcice.

### How to use Flint SDK
- Please see the reperence [here](http://matchstick.tv/developers/documents/get-started-to-fling.html).

### Supported FlintXBMC platforms
- Android
- iOS(in progress)


