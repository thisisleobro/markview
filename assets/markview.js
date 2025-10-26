document.addEventListener('keydown', async function(event) {
	console.log('keydown')
	if (event.key === 'F11') {
		console.log('F11 key pressed');
		await markview_toggle_fullscreen()
	}
});


function applyCss(base64) {
	const style = document.createElement('style');

	style.textContent = atob(base64);
	
	document.head.appendChild(style);
}


function interceptClickEvent(e) {
	e.preventDefault();
	
	const target = e.target || e.srcElement;
	let href = '';

	if (target.tagName === 'A') {
		href = target.getAttribute('href');
	}

	if (target.tagName === 'IMG') {
		href = target.parentElement.getAttribute('href');
	}

	if (!href) {
		return
	}

	if (href.endsWith('.md')) {
		markview_open_file(href.startsWith('file://')? href.slice(7): href)
	}

	// TODO ask system to open it
}

if (document.addEventListener) {
	document.addEventListener('click', interceptClickEvent);
} else if (document.attachEvent) {
	document.attachEvent('onclick', interceptClickEvent);
}

// hide the webview widget so we can capture events
document.addEventListener("dragover", (e) => {
	e.preventDefault()
	markview_hide_webview()
})

document.addEventListener("drop", (e) => e.preventDefault());
