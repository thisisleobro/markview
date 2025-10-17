document.addEventListener('keydown', async function(event) {
	if (event.key === 'F11') {
		console.log('F11 key pressed');
		await markview_toggle_fullscreen()
	}
});


function interceptClickEvent(e) {
	e.preventDefault();
	
	const target = e.target || e.srcElement;
	let href = '';


	if (target.tagName === 'A') {
		href = target.getAttribute('href');
	}
	else if(target.tagName === 'IMG') {
		href = target.parentElement.getAttribute('href');
	}

	if(href.startsWith('http') 
		&& !href.startsWith('http://localhost')
		&& !href.startsWith('http://127.0.0.1')
	) {
		alert(href)
		// openExternalLink(href);
	}
}

if (document.addEventListener) {
	document.addEventListener('click', interceptClickEvent);
} else if (document.attachEvent) {
	document.attachEvent('onclick', interceptClickEvent);
}