const joystick = document.getElementById("joystick");
const joystickContainer = document.querySelector(".joystick-container");
const slider1 = document.getElementById("slider1");
const slider2 = document.getElementById("slider2");
const toggleReturn = document.getElementById("toggle-return");
const statusLabel = document.getElementById("status");
const toggleTheme = document.getElementById("toggle-theme");

let joystickOffset = { x: 0, y: 0 };
let isDragging = false;
let xPos = 0;
let yPos = 0;

let socket;

// Initialize WebSocket
function initWebSocket() {
  socket = new WebSocket('ws://' + window.location.hostname + ':81/');

  socket.onopen = function () {
    console.log("WebSocket connection opened");
    updateStatus(true);
  };

  socket.onmessage = function (event) {
    console.log("Message from server: ", event.data);
    updateDataBox(event.data);
  };

  socket.onclose = function () {
    console.log("WebSocket connection closed");
    updateStatus(false);
  };

  socket.onerror = function (error) {
    console.error("WebSocket error: ", error);
    updateStatus(false);
  };
}

function updateDataBox(data) {
  const dataBox = document.getElementById("dataBox");
  dataBox.textContent = data;
}
function sendJoystickData(x, y) {
  const data = JSON.stringify({
    type: "joystick",
    x: Math.round(x * 125),
    y: Math.round(y * 125),
  });
  socket.send(data);
}

function sendSlider1Data() {
  const data = JSON.stringify({ slider1: slider1.value });
  document.getElementById("labelslider1").innerHTML =
    "Slider 1: " + slider1.value;
  socket.send(data);
}

function sendSlider2Data() {
  const data = JSON.stringify({ slider2: slider2.value });
  document.getElementById("labelslider2").innerHTML =
    "Slider 2: " + slider2.value;
  socket.send(data);
}

function initSliders() {
  document.getElementById("slider1").value = 0;
  document.getElementById("slider2").value = 0;
}
initSliders();

function toggleSwitch(id) {
  const button = document.getElementById("switch" + id);
  const isOn = button.classList.toggle("switch-on");
  const value = isOn ? 1 : 0;
  const data = JSON.stringify({ type: "switch", id: id, value: value });
  socket.send(data);
}
function sendPushData(id, value) {
  const button = document.getElementById("push" + id);
  const data = JSON.stringify({ type: "push", id: id, value: value });
  socket.send(data);
  if (value === 1) {
    button.classList.add("push-active");
  } else {
    button.classList.remove("push-active");
  }
}

joystick.addEventListener("mousedown", startDrag);
joystick.addEventListener("touchstart", startDrag, { passive: true });

function startDrag(event) {
  event.preventDefault();
  isDragging = true;
  joystick.style.transition = "left 0s, top 0s";

  document.addEventListener("mousemove", dragJoystick);
  document.addEventListener("mouseup", stopDrag);
  document.addEventListener("touchmove", dragJoystick, { passive: true });
  document.addEventListener("touchend", stopDrag);

  const rect = joystickContainer.getBoundingClientRect();

  if (event.touches) {
    joystickOffset.x = event.touches[0].clientX - rect.left - xPos;
    joystickOffset.y = event.touches[0].clientY - rect.top - yPos;
  } else {
    joystickOffset.x = event.clientX - rect.left - xPos;
    joystickOffset.y = event.clientY - rect.top - yPos;
  }
}

function dragJoystick(event) {
  if (!isDragging) return;

  const rect = joystickContainer.getBoundingClientRect();
  const limitRadius = rect.width / 2 - joystick.offsetWidth / 2;

  if (event.touches) {
    xPos = event.touches[0].clientX - rect.left - joystickOffset.x;
    yPos = event.touches[0].clientY - rect.top - joystickOffset.y;
  } else {
    xPos = event.clientX - rect.left - joystickOffset.x;
    yPos = event.clientY - rect.top - joystickOffset.y;
  }

  const distance = Math.sqrt(xPos * xPos + yPos * yPos);
  if (distance > limitRadius) {
    const angle = Math.atan2(yPos, xPos);
    xPos = limitRadius * Math.cos(angle);
    yPos = limitRadius * Math.sin(angle);
  }

  joystick.style.left = `${xPos + rect.width / 2}px`;
  joystick.style.top = `${yPos + rect.height / 2}px`;

  sendJoystickData(xPos / limitRadius, yPos / limitRadius);
}

function stopDrag() {
  isDragging = false;
  document.removeEventListener("mousemove", dragJoystick);
  document.removeEventListener("mouseup", stopDrag);
  document.removeEventListener("touchmove", dragJoystick);
  document.removeEventListener("touchend", stopDrag);
  joystick.style.transition = "left 0.4s, top 0.4s";
  if (toggleReturn.checked) {
    sendJoystickData(0, 0);
    xPos = 0;
    yPos = 0;
    joystick.style.left = "50%";
    joystick.style.top = "50%";
  }
}

function updateStatus(isConnected) {
  if (isConnected) {
    statusLabel.textContent = "Connected";
    statusLabel.classList.remove("status-disconnected");
    statusLabel.classList.add("status-connected");
  } else {
    statusLabel.textContent = "Disconnected";
    statusLabel.classList.remove("status-connected");
    statusLabel.classList.add("status-disconnected");
  }
}

function handleToggleChange() {
  if (toggleReturn.checked) {
    sendJoystickData(0, 0);
    xPos = 0;
    yPos = 0;
    joystick.style.left = "50%";
    joystick.style.top = "50%";
  } else {
    console.log("Return to Home is disabled");
  }
}

function handleSL() {
  if (document.getElementById("toggle-sl").checked) {
    document.querySelector(".container").style.touchAction = "none";
  } else {
    document.querySelector(".container").style.touchAction = "auto";
  }
}
function handleThemeToggle() {
  const body = document.body;
  const container = document.querySelector(".container");
  const themeToggle = document.getElementById("theme-toggle");
  const isDarkMode = body.classList.toggle("dark-mode");
  if (isDarkMode) {
    // Dark mode
    themeToggle.classList.add("dark-mode");
    container.classList.add("dark-mode");
    body.classList.add("dark-mode");
    document.querySelector(".joystick-container").classList.add("dark-mode");
    document.querySelector(".joystick").classList.add("dark-mode");
    document.querySelector(".data-box").classList.add("dark-mode");
    document.querySelector(".arrow").classList.add("dark-mode");
    document.querySelector(".status-label").classList.add("dark-mode");
    // update all the arrows with the class arrow
    document.querySelectorAll(".arrow").forEach((element) => {
      element.classList.add("dark-mode");
    });
    // update all the buttons with the class button
    document.querySelectorAll(".button").forEach((element) => {
      element.classList.add("dark-mode");
    });
    // Update all elements with the common-label class
    document.querySelectorAll(".common-label").forEach((element) => {
      element.classList.add("dark-mode");
    });
  } else {
    // Light mode
    themeToggle.classList.remove("dark-mode");
    body.classList.remove("dark-mode");
    container.classList.remove("dark-mode");
    document.querySelector(".joystick-container").classList.remove("dark-mode");
    document.querySelector(".joystick").classList.remove("dark-mode");
    document.querySelector(".data-box").classList.remove("dark-mode");
    document.querySelector(".arrow").classList.remove("dark-mode");
    document.querySelector(".status-label").classList.remove("dark-mode");
    // update all the arrows with the class arrow
    document.querySelectorAll(".arrow").forEach((element) => {
      element.classList.remove("dark-mode");
    });
    // update all the buttons with the class button
    document.querySelectorAll(".button").forEach((element) => {
      element.classList.remove("dark-mode");
    });
    // Update all elements with the common-label class
    document.querySelectorAll(".common-label").forEach((element) => {
      element.classList.remove("dark-mode");
    });

    // Save preference to local storage
    localStorage.setItem("darkMode", isDarkMode);
  }
}
// Initialize theme based on local storage
window.onload = function () {
  const isDarkMode = localStorage.getItem("darkMode") === "true";
  if (isDarkMode) {
    document.body.classList.add("dark-mode");
    document.getElementById("theme-toggle").classList.add("dark-mode");
  }
};
// window.alert("sometext");
//alert("By defult, the return to home is enabled. If you want to disable it, uncheck the RTH checkbox. The SL checkbox is for disabling the touch action of the screen. If you want to enable the touch action, uncheck the SL checkbox.");
window.onload = initWebSocket;
