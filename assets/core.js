// https://stackoverflow.com/questions/30106476/using-javascripts-atob-to-decode-base64-doesnt-properly-decode-utf-8-strings
function decodeBase64(base64) {
	const text = atob(base64);
	const length = text.length;
	const bytes = new Uint8Array(length);
	for (let i = 0; i < length; i++) {
		bytes[i] = text.charCodeAt(i);
	}
	const decoder = new TextDecoder(); // default is utf-8
	return decoder.decode(bytes);
}

function markview_api_apply_css_from_base64(base64Content) {
	const style = document.createElement('style');

	style.textContent = decodeBase64(base64Content);

	document.head.appendChild(style);
}

function markview_api_render_html(base64Content) {
	const decodedContent = decodeBase64(base64Content);

	if (typeof DOMPurify === 'undefined') {
		window.root.innerHTML = '<h1>No DOMPurify found. Could be vulnerable to script injection. Please contact the author</h1>'
	} else {
		const cleanHtml = DOMPurify.sanitize(decodedContent)
		window.root.innerHTML = cleanHtml

		Prism.highlightAll()
	}
}