# CloudLogOffline

[![CloudLogOfflineDemo](http://img.youtube.com/vi/fIP1E9rO_GM/0.jpg)](http://www.youtube.com/watch?v=fIP1E9rO_GM "CloudLogOffline Demo")

CloudLogOffline is an app interface for [Cloudlog](https://github.com/magicbug/Cloudlog) the cloud logbook for HAM radio OMs and YLs by 2M0SQL.

The main purpose of CloudLogOffline is the portable operating mode, where no Wifi or 3G/LTE is availabe, e.g. SOTA, IOTA or COTA. The logs can be stored in the app and when back to a internet connection, the log can be uploaded to a selfhosted Cloudlog instance. This app is developed as cross-plattform tool for macOS, iOS, iPadOS, Android, Windows, Linux using the Qt framework.

Currently CloudLogOffline supports following features:

- Upload to Cloudlog via API 
- Query QRZ.com (if 3G/LTE is available)
- Connect to [Flrig](http://www.w1hkj.com) by W1HKJ which e.g. runs on a Raspberry Pi which is connected to the radio and opens a Wifi to interact with CloudLogOffline
- Set a CQ QRG if Flrig is not available
- Live and post QSOs