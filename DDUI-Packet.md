# Minecraft Bedrock DDUI Packet

## v26.10

### CustomForm

```log
ClientboundDataDrivenUIShowScreenPacket：{"ScreenId":"minecraft:message_box","FormId":4}
ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"message_box_data","Update Count":3,"The New Property Value":{"body":"Msg Body","button1":{"label":"Btn1","onClick":0,"tooltip":"Btn1 Tooltip"},"button2":{"label":"Btn2","onClick":0,"tooltip":"Btn2 Tooltip"},"title":"Msg Title"}},{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":7,"The New Property Value":true}]}
ClientboundDataDrivenUICloseScreenPacket：{"FormId":4}
ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":8,"The New Property Value":false},{"Data Store Name":"minecraft","Property":"message_box_data","Update Count":4,"The New Property Value":null}]}

ClientboundDataDrivenUIShowScreenPacket：{"ScreenId":"minecraft:message_box","FormId":5}
ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"message_box_data","Update Count":5,"The New Property Value":{"body":"Msg Body","button1":{"label":"Btn1","onClick":0,"tooltip":"Btn1 Tooltip"},"button2":{"label":"Btn2","onClick":0,"tooltip":"Btn2 Tooltip"},"title":"Msg Title"}},{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":9,"The New Property Value":true}]}
ClientboundDataDrivenUICloseScreenPacket：{"FormId":5}
ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":10,"The New Property Value":false},{"Data Store Name":"minecraft","Property":"message_box_data","Update Count":6,"The New Property Value":null}]}
```

### MessageBox

```log
ClientboundDataDrivenUIShowScreenPacket：{"ScreenId":"minecraft:custom_form","FormId":6}
ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"custom_form_data","Update Count":5,"The New Property Value":{"closeButton":{"button_visible":true,"label":"Close","onClick":0},"layout":{"0":{"header_visible":true,"visible":true,"text":"--- 控件穷举开始 ---"},"1":{"label_visible":true,"text":"这是一个 Label","visible":true},"2":{"divider_visible":true,"visible":true},"3":{"spacer_visible":true,"visible":true},"4":{"description":"Slider 描述","slider_visible":true,"label":"Slider","disabled":false,"maxValue":100,"minValue":0,"step":2,"value":50,"visible":true},"5":{"description":"Dropdown 描述","dropdown_visible":true,"visible":true,"disabled":false,"label":"Dropdown","items":{"0":{"description":"Desc 0","label":"Item 0","value":0},"1":{"description":"Desc 1","label":"Item 1","value":1},"length":2},"value":0},"6":{"description":"TextField 描述","visible":true,"disabled":false,"label":"TextField","text":"init text","textfield_visible":true},"7":{"description":"Toggle 描述","visible":true,"disabled":false,"label":"Toggle","toggle_visible":true,"toggled":true},"8":{"button_visible":true,"disabled":false,"label":"Button","onClick":0,"tooltip":"按钮 Tooltip","visible":true},"length":9},"title":"Super Dynamic Title"}},{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":11,"The New Property Value":true}]}
ClientboundDataDrivenUICloseScreenPacket：{"FormId":6}
ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":12,"The New Property Value":false},{"Data Store Name":"minecraft","Property":"custom_form_data","Update Count":6,"The New Property Value":null}]}
ClientboundDataDrivenUIShowScreenPacket：{"ScreenId":"minecraft:custom_form","FormId":7}
ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"custom_form_data","Update Count":7,"The New Property Value":{"closeButton":{"button_visible":true,"label":"Close","onClick":0},"layout":{"0":{"header_visible":true,"visible":true,"text":"--- 控件穷举开始 ---"},"1":{"label_visible":true,"text":"这是一个 Label","visible":true},"2":{"divider_visible":true,"visible":true},"3":{"spacer_visible":true,"visible":true},"4":{"description":"Slider 描述","slider_visible":true,"label":"Slider","disabled":false,"maxValue":100,"minValue":0,"step":2,"value":68,"visible":true},"5":{"description":"Dropdown 描述","dropdown_visible":true,"visible":true,"disabled":false,"label":"Dropdown","items":{"0":{"description":"Desc 0","label":"Item 0","value":0},"1":{"description":"Desc 1","label":"Item 1","value":1},"length":2},"value":0},"6":{"description":"TextField 描述","visible":true,"disabled":false,"label":"TextField","text":"init text","textfield_visible":true},"7":{"description":"Toggle 描述","visible":true,"disabled":false,"label":"Toggle","toggle_visible":true,"toggled":true},"8":{"button_visible":true,"disabled":false,"label":"Button","onClick":0,"tooltip":"按钮 Tooltip","visible":true},"length":9},"title":"Super Dynamic Title"}},{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":13,"The New Property Value":true}]}
ClientboundDataDrivenUICloseScreenPacket：{"FormId":7}
ClientboundDataStorePacket：{"Updates":[{"Data Store Name":"minecraft","Property":"ddui_form_active","Update Count":14,"The New Property Value":false},{"Data Store Name":"minecraft","Property":"custom_form_data","Update Count":8,"The New Property Value":null}]}
```

### Interface

```ts
interface ClientboundDataDrivenUIShowScreenPacket {
  ScreenId: "minecraft:custom_form" | "minecraft:message_box";
  FormId: number;
}

interface ClientboundDataDrivenUICloseScreenPacket {
  FormId: number;
}

// ---------- Message Box ----------

interface MessageButton {
  label: string;
  onClick: 0;
  tooltip: string;
}

interface MessageBoxData {
  body: string;
  button1: MessageButton;
  button2: MessageButton;
  title: string;
}

// ---------- Custom Form ----------

interface CloseButton {
  button_visible: boolean;
  label: string;
  onClick: 0;
}

// --- Layout items ---

interface HeaderItem {
  header_visible: boolean;
  visible: boolean;
  text: string;
}

interface LabelItem {
  label_visible: boolean;
  text: string;
  visible: boolean;
}

interface DividerItem {
  divider_visible: boolean;
  visible: boolean;
}

interface SpacerItem {
  spacer_visible: boolean;
  visible: boolean;
}

interface SliderItem {
  description: string;
  slider_visible: boolean;
  label: string;
  disabled: boolean;
  maxValue: number;
  minValue: number;
  step: number;
  value: number;
  visible: boolean;
}

interface DropdownItemOption {
  description: string;
  label: string;
  value: number;
}

type DropdownItems = { length: number } & Record<string, DropdownItemOption>;

interface DropdownItem {
  description: string;
  dropdown_visible: boolean;
  visible: boolean;
  disabled: boolean;
  label: string;
  items: DropdownItems;
  value: number;
}

interface TextFieldItem {
  description: string;
  visible: boolean;
  disabled: boolean;
  label: string;
  text: string;
  textfield_visible: boolean;
}

interface ToggleItem {
  description: string;
  visible: boolean;
  disabled: boolean;
  label: string;
  toggle_visible: boolean;
  toggled: boolean;
}

interface ButtonItem {
  button_visible: boolean;
  disabled: boolean;
  label: string;
  onClick: 0;
  tooltip: string;
  visible: boolean;
}

type LayoutItem =
  | HeaderItem
  | LabelItem
  | DividerItem
  | SpacerItem
  | SliderItem
  | DropdownItem
  | TextFieldItem
  | ToggleItem
  | ButtonItem;

type Layout = { length: number } & Record<string, LayoutItem>; // key: Number(index).toString()

interface CustomFormData {
  closeButton: CloseButton;
  layout: Layout;
  title: string;
}

// ---------- Data Store Packet ----------

type ClientboundDataStoreUpdate =
  | {
      "Data Store Name": "minecraft";
      Property: "ddui_form_active";
      "Update Count": number;
      "The New Property Value": boolean;
    }
  | {
      "Data Store Name": "minecraft";
      Property: "message_box_data";
      "Update Count": number;
      "The New Property Value": MessageBoxData | null;
    }
  | {
      "Data Store Name": "minecraft";
      Property: "custom_form_data";
      "Update Count": number;
      "The New Property Value": CustomFormData | null;
    };

interface ClientboundDataStorePacket {
  Updates: ClientboundDataStoreUpdate[];
}
```
