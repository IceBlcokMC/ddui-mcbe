# Minecraft Bedrock DDUI Packet

## v26.10

### MessageBox

#### Server

```log
[1778573443801][Send] ClientboundDataDrivenUIShowScreenPacket：{"ScreenId":"minecraft:message_box","FormId":1}
[1778573443845][Send] ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"message_box_data","Update Count":1,"The New Property Value":{"body":"Msg Body","button1":{"label":"Btn1","onClick":0,"tooltip":"Btn1 Tooltip"},"button2":{"label":"Btn2","onClick":0,"tooltip":"Btn2 Tooltip"},"title":"Msg Title"}},{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":1,"The New Property Value":true}]}
[1778573446121][Receive] ServerboundDataStorePacket：{"Update":{"Data Store Name":"minecraft","Property":"message_box_data","Path":"button1.onClick","Data":1.0,"Property Update Count":1,"Path Update Count":1}}
[1778573446125][Send] ClientboundDataDrivenUICloseScreenPacket：{"FormId":1}
[1778573446168][Receive] ServerboundDataDrivenScreenClosedPacket：{"FormId":1,"CloseReason":"programmaticclose"}
[1778573446316][Send] ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":2,"The New Property Value":false},{"Data Store Name":"minecraft","Property":"message_box_data","Update Count":2,"The New Property Value":null}]}
```

#### Client

```log
[1778573443825][Receive] ClientboundDataDrivenUIShowScreenPacket：{"ScreenId":"minecraft:message_box","FormId":1}
[1778573444080][Receive] ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"message_box_data","Update Count":1,"The New Property Value":{"body":"Msg Body","button1":{"label":"Btn1","tooltip":"Btn1 Tooltip","onClick":0},"title":"Msg Title","button2":{"label":"Btn2","tooltip":"Btn2 Tooltip","onClick":0}}},{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":1,"The New Property Value":true}]}
[1778573446095][Send] ServerboundDataStorePacket：{"Update":{"Data Store Name":"minecraft","Property":"message_box_data","Path":"button1.onClick","Data":1.0,"Property Update Count":1,"Path Update Count":1}}
[1778573446140][Receive] ClientboundDataDrivenUICloseScreenPacket：{"FormId":1}
[1778573446141][Send] ServerboundDataDrivenScreenClosedPacket：{"FormId":1,"CloseReason":"programmaticclose"}
[1778573446341][Receive] ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":2,"The New Property Value":false},{"Data Store Name":"minecraft","Property":"message_box_data","Update Count":2,"The New Property Value":null}]}
```

### CustomForm

#### Server

```log
[1778573563079][Send] ClientboundDataDrivenUIShowScreenPacket：{"ScreenId":"minecraft:custom_form","FormId":2}
[1778573563118][Send] ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"custom_form_data","Update Count":1,"The New Property Value":{"closeButton":{"button_visible":true,"label":"Close","onClick":0},"layout":{"0":{"header_visible":true,"visible":true,"text":"--- 控件穷举开始 ---"},"1":{"label_visible":true,"text":"这是一个 Label","visible":true},"2":{"divider_visible":true,"visible":true},"3":{"spacer_visible":true,"visible":true},"4":{"description":"Slider 描述","slider_visible":true,"label":"Slider","disabled":false,"maxValue":100,"minValue":0,"step":2,"value":50,"visible":true},"5":{"description":"Dropdown 描述","dropdown_visible":true,"visible":true,"disabled":false,"label":"Dropdown","items":{"0":{"description":"Desc 0","label":"Item 0","value":0},"1":{"description":"Desc 1","label":"Item 1","value":1},"length":2},"value":0},"6":{"description":"TextField 描述","visible":true,"disabled":false,"label":"TextField","text":"init text","textfield_visible":true},"7":{"description":"Toggle 描述","visible":true,"disabled":false,"label":"Toggle","toggle_visible":true,"toggled":true},"8":{"button_visible":true,"disabled":false,"label":"RandomData","onClick":0,"tooltip":"按钮 Tooltip","visible":true},"length":9},"title":"Super Dynamic Title"}},{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":3,"The New Property Value":true}]}
[1778573566581][Receive] ServerboundDataStorePacket：{"Update":{"Data Store Name":"minecraft","Property":"custom_form_data","Path":"closeButton.onClick","Data":1.0,"Property Update Count":1,"Path Update Count":1}}
[1778573566584][Send] ClientboundDataDrivenUICloseScreenPacket：{"FormId":2}
[1778573566780][Receive] ServerboundDataDrivenScreenClosedPacket：{"FormId":2,"CloseReason":"programmaticclose"}
[1778573566873][Send] ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":4,"The New Property Value":false},{"Data Store Name":"minecraft","Property":"custom_form_data","Update Count":2,"The New Property Value":null}]}
```

#### Client

```log
[1778573563095][Receive] ClientboundDataDrivenUIShowScreenPacket：{"ScreenId":"minecraft:custom_form","FormId":2}
[1778573563299][Receive] ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"custom_form_data","Update Count":1,"The New Property Value":{"closeButton":{"button_visible":true,"label":"Close","onClick":0},"layout":{"0":{"visible":true,"header_visible":true,"text":"--- 控件穷举开始 ---"},"1":{"label_visible":true,"text":"这是一个 Label","visible":true},"length":9,"2":{"divider_visible":true,"visible":true},"3":{"spacer_visible":true,"visible":true},"4":{"description":"Slider 描述","disabled":false,"slider_visible":true,"label":"Slider","maxValue":100,"minValue":0,"step":2,"value":50,"visible":true},"5":{"visible":true,"dropdown_visible":true,"description":"Dropdown 描述","label":"Dropdown","disabled":false,"items":{"0":{"description":"Desc 0","label":"Item 0","value":0},"1":{"description":"Desc 1","label":"Item 1","value":1},"length":2},"value":0},"6":{"visible":true,"description":"TextField 描述","label":"TextField","disabled":false,"text":"init text","textfield_visible":true},"7":{"visible":true,"description":"Toggle 描述","label":"Toggle","disabled":false,"toggle_visible":true,"toggled":true},"8":{"button_visible":true,"label":"RandomData","disabled":false,"tooltip":"按钮 Tooltip","onClick":0,"visible":true}},"title":"Super Dynamic Title"}},{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":3,"The New Property Value":true}]}
[1778573566542][Send] ServerboundDataStorePacket：{"Update":{"Data Store Name":"minecraft","Property":"custom_form_data","Path":"closeButton.onClick","Data":1.0,"Property Update Count":1,"Path Update Count":1}}
[1778573566614][Receive] ClientboundDataDrivenUICloseScreenPacket：{"FormId":2}
[1778573566614][Send] ServerboundDataDrivenScreenClosedPacket：{"FormId":2,"CloseReason":"programmaticclose"}
[1778573566894][Receive] ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":4,"The New Property Value":false},{"Data Store Name":"minecraft","Property":"custom_form_data","Update Count":2,"The New Property Value":null}]}
```
