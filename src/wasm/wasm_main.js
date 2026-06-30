// ======================= Start of wasm_main.js =======================
/*
File:   wasm_main.js
Author: Taylor Robbins
Date:   01\10\2025
Description:
	** Contains the main client-side code for the test application
*/

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
function AcquireCanvas(canvasWidth, canvasHeight)
{
	canvas = document.getElementsByTagName("canvas")[0];
	// console.log(canvas);
	
	// set the display size of the canvas.
	canvas.style.width = canvasWidth + "px";
	canvas.style.height = canvasHeight + "px";
	
	// set the size of the drawingBuffer
	var devicePixelRatio = window.devicePixelRatio || 1;
	canvas.width = canvasWidth * devicePixelRatio;
	canvas.height = canvasHeight * devicePixelRatio;
	
	// canvasContainer = document.getElementById("canvas_container");
	// console.assert(canvasContainer != null, "Couldn't find canvas container DOM element!");
	appGlobals.canvas = canvas;
	return canvas;
}

function CreateGlContext(canvas)
{
	canvasContextGl = canvas.getContext("webgl2");
	if (canvasContextGl === null) { console.error("Unable to initialize WebGL render context. Your browser or machine may not support it :("); return null; }
	// console.log(canvasContextGl);
	appGlobals.glContext = canvasContextGl;
	return canvasContextGl;
}

async function InitializeWasmModule(wasmFilePath, initialWasmPageCount)
{
	appGlobals.textDecoder = new TextDecoder("utf-8");
	// appGlobals.wasmMemory = new WebAssembly.Memory({ initial: initialWasmPageCount });
	
	let wasmEnvironment =
	{
		...jsStdApiFuncs,
		...jsAppApiFuncs,
	};
	appGlobals.wasmModule = await loadWasmModule(wasmFilePath, wasmEnvironment);
	
	appGlobals.memDataView = new DataView(new Uint8Array(appGlobals.wasmModule.exports.memory.buffer).buffer);
	let heapBaseAddress = appGlobals.wasmModule.exports.__heap_base.value;
	console.log("__heap_base = " + heapBaseAddress);
	
	let memorySize = appGlobals.wasmModule.exports.memory.buffer.byteLength;
	let numMemoryPagesAfterLoad = memorySize / WASM_MEMORY_PAGE_SIZE;
	if ((memorySize % WASM_MEMORY_PAGE_SIZE) != 0)
	{
		console.warn("memorySize (" + memorySize + ") is not a multiple of WASM_MEMORY_PAGE_SIZE (" + WASM_MEMORY_PAGE_SIZE + ")");
		numMemoryPagesAfterLoad++;
	}
	appGlobals.wasmModule.exports.InitStdLib(numMemoryPagesAfterLoad);
	
	let initializeTimestamp = Math.floor(Date.now() / 1000); //TODO: Should we be worried about this being a 32-bit float?
	appGlobals.wasmModule.exports.ModuleInit(initializeTimestamp);
}

// +--------------------------------------------------------------+
// |                           MainLoop                           |
// +--------------------------------------------------------------+
async function MainLoop()
{
	// console.log("MainLoop start...");
	canvas = AcquireCanvas(800, 600);
	glContext = CreateGlContext(canvas);
	
	// console.log("Calling init...");
	await InitializeWasmModule("app.wasm", 4);
	
	// window.addEventListener("mousemove", function(event)
	// {
	// 	let clientBounds = canvas.getBoundingClientRect();
	// 	let pixelRatio = window.devicePixelRatio;
	// 	mousePositionX = Math.round(event.clientX - clientBounds.left) * pixelRatio;
	// 	mousePositionY = Math.round(event.clientY - clientBounds.top) * pixelRatio;
	// });
	// window.addEventListener("keydown", function(event)
	// {
	// 	let key = KeyDownEventStrToKeyEnum(event.code);
	// 	appGlobals.wasmModule.exports.HandleKeyPressOrRelease(key, true);
	// });
	// window.addEventListener("keyup", function(event)
	// {
	// 	let key = KeyDownEventStrToKeyEnum(event.code);
	// 	appGlobals.wasmModule.exports.HandleKeyPressOrRelease(key, false);
	// });
	// window.addEventListener("mousedown", function(event)
	// {
	// 	let mouseBtn = MouseDownEventNumToBtnEnum(event.button);
	// 	appGlobals.wasmModule.exports.HandleMousePressOrRelease(mouseBtn, true);
	// });
	// window.addEventListener("mouseup", function(event)
	// {
	// 	let mouseBtn = MouseDownEventNumToBtnEnum(event.button);
	// 	appGlobals.wasmModule.exports.HandleMousePressOrRelease(mouseBtn, false);
	// });
	
	// appGlobals.wasmModule.exports.UpdateAndRender(0);
	
	function renderFrame()
	{
		appGlobals.wasmModule.exports.ModuleUpdate(16.6666);
		window.requestAnimationFrame(renderFrame);
	}
	window.requestAnimationFrame(renderFrame);
	// console.log("MainLoop Done!");
}

MainLoop();

// ======================== End of wasm_main.js ========================