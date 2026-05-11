import { world, Player } from "@minecraft/server";
import { CustomForm, MessageBox, Observable, UIRawMessage } from "@minecraft/server-ui";

const observables: any[] = [];

function obs<T extends string | number | boolean | UIRawMessage>(val: T): Observable<T> {
  const o = Observable.create(val, { clientWritable: true });
  observables.push(o);
  return o;
}

const titleObs = obs("Super Dynamic Title") as Observable<string>;
const headerText = obs("--- 控件穷举开始 ---") as Observable<string>;
const labelText = obs("这是一个 Label") as Observable<string>;
const sliderVal = obs(50) as Observable<number>;
const sliderMin = obs(0) as Observable<number>;
const sliderMax = obs(100) as Observable<number>;
const sliderStep = obs(2) as Observable<number>;
const sliderDesc = obs("Slider 描述") as Observable<string>;
const dropdownVal = obs(0) as Observable<number>;
const dropdownDesc = obs("Dropdown 描述") as Observable<string>;
const textFieldText = obs("init text") as Observable<string>;
const textFieldDesc = obs("TextField 描述") as Observable<string>;
const toggleVal = obs(true) as Observable<boolean>;
const toggleDesc = obs("Toggle 描述") as Observable<string>;
const btnTooltip = obs("按钮 Tooltip") as Observable<string>;

function giantCustomForm(player: Player) {
  const form = CustomForm.create(player, titleObs);

  form.closeButton();
  form.header(headerText);
  form.label(labelText);

  // Divider
  form.divider();

  // Spacer
  form.spacer();

  // Slider
  form.slider("Slider", sliderVal, sliderMin, sliderMax, {
    step: sliderStep,
    disabled: false,
    visible: true,
    description: sliderDesc,
  });

  // Dropdown
  form.dropdown(
    "Dropdown",
    dropdownVal,
    [
      { label: "Item 0", value: 0, description: "Desc 0" },
      { label: "Item 1", value: 1, description: "Desc 1" },
    ],
    {
      disabled: false,
      visible: true,
      description: dropdownDesc,
    }
  );

  // TextField
  form.textField("TextField", textFieldText, {
    disabled: false,
    visible: true,
    description: textFieldDesc,
  });

  // Toggle
  form.toggle("Toggle", toggleVal, {
    disabled: false,
    visible: true,
    description: toggleDesc,
  });

  // Button
  form.button(
    "RandomData",
    () => {
      let ranHex = Math.floor(Math.random() * 16777215).toString(16);
      titleObs.setData(ranHex);
      headerText.setData(ranHex);
      labelText.setData(ranHex);
      sliderVal.setData(Math.floor(Math.random() * 100));
      dropdownVal.setData(Math.floor(Math.random() * 2));
      textFieldText.setData(ranHex);
      toggleVal.setData(Math.random() > 0.5);
    },
    {
      disabled: false,
      visible: true,
      tooltip: btnTooltip,
    }
  );

  form.show();
}

// ==========================================
// MessageBox
// ==========================================
const msgTitle = obs("Msg Title") as Observable<string>;
const msgBody = obs("Msg Body") as Observable<string>;
const msgBtn1Label = obs("Btn1") as Observable<string>;
const msgBtn2Label = obs("Btn2") as Observable<string>;
const msgBtn1Tooltip = obs("Btn1 Tooltip") as Observable<string>;
const msgBtn2Tooltip = obs("Btn2 Tooltip") as Observable<string>;

function giantMessageBox(player: Player) {
  const msg = MessageBox.create(player, msgTitle);
  msg.body(msgBody);
  msg.button1(msgBtn1Label, msgBtn1Tooltip);
  msg.button2(msgBtn2Label, msgBtn2Tooltip);
  msg.show();
}

world.afterEvents.playerPlaceBlock.subscribe((event) => {
  const p = event.player;
  if (event.block.type.id == "minecraft:grass_block") {
    giantCustomForm(p);
  } else {
    giantMessageBox(p);
  }
});
