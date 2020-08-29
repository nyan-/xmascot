! XMascot Resource File

!*gravity: 163
!*chainLen: 70
!*dampCoeff: 0.01
!*degree: 30
!*magnifyiBase: 1.0
!*action: sound(pipipipi.au) start_move
!*search:
!*pinPattern: pin.gif
!*pcol0: -1
!*prgb0: auto
!*chainNum: 6
!*chanePattern: chain.gif
!*ccol0: -1
!*crgb0: auto
!*biff: true
!*biffOnce: true
!*update: 30
!*biffCmd:
!*biffAction: start_move bell(100)
!*biffPattern: mail_r.gif
!*bcol0: -1
!*brgb0: auto
!*biffPos: right
!*biffFilter: nkf -e -m
!*biffPopdown: 10
!*shadow: 4

*soundCommand:   SOUND_COMMAND_NAME
*bitmapFilePath: XMASDIR

*cursor_normal: yubi.xbm
*cursor_click:  osu.xbm
*cursor_drag:   tumamu.xbm

*ok.label: OK
*cancel.label:CANCEL

*popup.title: XMascot Menu

*popup.label: XMascot Menu
*popup.change.label: Change Mascot
*popup.preference.label: Change Parameter
*popup.alarm.label: Set Alarm/Chime
*popup.list.label: Arrived Mail lists
*popup.about.label: About Xmascot
*popup.exit.label: Exit Program

*changeBase.title: XMascot Change Mascot
*change.label: Change Mascot

*aboutBase.title: About XMascot
*about.Label.borderWidth: 0
*about.logo.file: logo.gif
*about.label1.label: About XMascot Ver2.6
*about.label2.label: \
Programed by Go Watanabe/Tsuyoshi Iida (TUT Computer Club)\n\
Copyright 1996,1997\n\
\n\
XMascot is a Program to Disp Funny-Pretty Mascot on your X.\n\
Please keep it on your screen :-)

*preferenceBase.title: XMascot Preference
*preference*Label.borderWidth: 0
*preference*Box.borderWidth: 0
*preference*Scrollbar.length: 100

*preference*title.label: Change Parameter
*preference*grav_label.label: Gravity
*preference*damp_label.label: Dumping
*preference*shadow_label.label: Shadow Length
*preference*mag_label.label: Magnify
*preference*chain_label.label: Chains number

*alarmBase.title: XMascot Alarm Dialog
*alarm*Label.borderWidth: 0
*alarm*Box.borderWidth: 0

*alarm*Text*translations: #override <Key>Return: end-of-line()
*alarm*hour.width: 30
*alarm*min.width: 30
*alarm*action.width: 200
*alarm*action*scrollHorizontal: always
*alarm*test.label: TEST

*alarm.Label.justify: left
*alarm.title.label: Setup Alarm/Chime
*alarm.alarm.label: Daily Alarm hh:mm
*alarm.timer.label: Timer Alarm mm:ss
*alarm.interval.label: Interval Timer hh:mm
*alarm.hourchime.label: Hour Chime
*alarm.halfchime.label: Half-Hour Chime
*alarm.biff.label: Biff Action

*biffList.geometry: 400x200+0+0
*biffList.title: XMascot Mail Lists

*biffList*title.label: Arrived Mail Lists
*biffList*title.borderWidth: 0

*biffList*title.top: ChainTop
*biffList*title.bottom: ChainTop
*biffList*title.left: ChainLeft
*biffList*title.right: ChainLeft

*biffList*text.fromVert: title
*biffList*text.top: ChainTop
*biffList*text.bottom: ChainBottom
*biffList*text.left: ChainLeft
*biffList*text.right: ChainRight
*biffList*text.scrollHorizontal: whenNeeded
*biffList*text.scrollVertical: whenNeeded

*biffList*ok.fromVert: text
*biffList*ok.top: ChainBottom
*biffList*ok.bottom: ChainBottom
*biffList*ok.left: ChainLeft
*biffList*ok.right: ChainLeft

*MasDat.rgb0: auto
*MasDat.col0: -1
*MasDat.magnify: 1.0
*MasDat.biffPos: right

!*menusNum: 1

*menu0.title: Originals
*menu0.numsOfMenu: 10

*menu0.masDat0.title: Kuma 
*menu0.masDat0.filename: kuma.gif

*menu0.masDat1.title: Wan Wan
*menu0.masDat1.filename: inu.gif

*menu0.masDat2.title: Nyan Nyan
*menu0.masDat2.filename: neko.gif

*menu0.masDat3.title: Nya-go
*menu0.masDat3.filename: neko2.gif

*menu0.masDat4.title: Teru Teru
*menu0.masDat4.filename: teru.gif

*menu0.masDat5.title: Penguin
*menu0.masDat5.filename: pen.gif

*menu0.masDat6.title: Osakana
*menu0.masDat6.filename: fish.gif

*menu0.masDat7.title: Osaru San
*menu0.masDat7.filename: saru.gif

*menu0.masDat8.title: Usagi San
*menu0.masDat8.filename: rabi.gif

*menu0.masDat9.title: Onnano ko
*menu0.masDat9.filename: monohosi.gif
