/**
 * @param {KeyboardEvent} event
 */
function handleKeypress(event) {
	if (event.key === 'F11') {
		console.log('F11 key pressed');
		markview_toggle_fullscreen()
	}
}

/**
 * @param {PointerEvent} e
 */
function interceptClickEvent(e) {
	const target = e.target || e.srcElement;
	let href = '';

	if (target.tagName === 'A') {
		href = target.getAttribute('href');
	} else if (target.tagName === 'IMG') {
		href = target.parentElement.getAttribute('href');
	}

	if (!href || href.startsWith('#')) {
		return
	}

	e.preventDefault();

	const fullUrl = new URL(href, 'file:');

	if (href.endsWith('.md') && (fullUrl.protocol === 'file:')) {
		markview_open_file(href.startsWith('file://') ? href.slice(7) : href)
	} else if (fullUrl.protocol == 'http:' || fullUrl.protocol == 'https:') {
		// TODO: ask program to open in defaylt browser
	}
}

document.addEventListener('click', interceptClickEvent);

// hide the webview widget so we can capture events
document.addEventListener("dragover", (e) => {
	e.preventDefault();
	if (e.target.tagName === 'HTML') {
		markview_hide_webview();
	}
})

document.addEventListener("drop", (e) => e.preventDefault());

// document.addEventListener("contextmenu", (event) => {
// 	event.preventDefault();
// })