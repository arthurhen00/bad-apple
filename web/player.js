const screen = document.getElementById("screen");
const frameElement = document.getElementById("frame");
const totalFramesElement = document.getElementById("total-frames");
const playButton = document.getElementById("play");
const audio = document.getElementById("audio");
const volumeControl = document.getElementById("volume-control");

let currentFrame = 0;
let playing = false;

let frameCount = 0;
let fps = 30;
let frameTime = 1000 / 30;

let startTime = 0;

function initializePlayer() {
  frameCount = Module._get_frame_count();
  fps = Module._get_fps();
  frameTime = 1000 / fps;

  totalFramesElement.textContent = frameCount;

  document.getElementById("loading").style.display = "none";

  playButton.disabled = false;

  playButton.addEventListener("click", start);

  volumeControl.addEventListener("input", function () {
    audio.volume = volumeControl.value;
    }
  );
}

function getFrame(frame) {
  const ptr = Module._get_frame(frame);

  if (!ptr) {
    return null;
  }

  return Module.UTF8ToString(ptr);
}

function renderFrame(frame) {
  const content = getFrame(frame);

  if (content === null) {
    console.error("Failed to load frame:", frame);
    return false;
  }

  screen.textContent = content;
  frameElement.textContent = frame;

  return true;
}

function start() {
  if (playing) {
    return;
  }

  playing = true;
  currentFrame = 1;

  renderFrame(currentFrame);

  audio.currentTime = 0;

  audio.play().catch(
    error => {
      console.error("Audio error:", error);
    }
  );

  startTime = performance.now();

  requestAnimationFrame(loop);
}

function loop(now) {
  if (!playing) {
    return;
  }

  const elapsed = now - startTime;
  const targetFrame = Math.floor(elapsed / frameTime) + 1;

  if (targetFrame > frameCount) {
    stop();
    return;
  }

  if (targetFrame !== currentFrame) {
    currentFrame = targetFrame;
    renderFrame(currentFrame);
  }

  requestAnimationFrame(loop);
}


function stop() {
  playing = false;
  audio.pause();
  audio.currentTime = 0;
}