#define INDEX_PAGE \
"<!DOCTYPE html>" \
"<html lang=\"en\">" \
"    <head>" \
"        <title>HTTP Server</title>" \
"    </head>" \
"    <body>" \
"        <img id=\"streamimage\" style=\"transform: scale(-1, 1)\" src=\"%s\">" \
"        <div id=\"brightness-ctrl\" style=\"display: flex; align-items: center; gap: 10px\">" \
"            <h3>Brightness:</h3>" \
"            <p id=\"brightness-value\">%d</p>" \
"            <div style=\"display: flex; align-items: center\">" \
"                <strong>0</strong>" \
"                <input id=\"brightness-input\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" />" \
"                <strong>255</strong>" \
"            </div>" \
"            <script>" \
"                const brightness_value = document.getElementById(\"brightness-value\");" \
"                brightness_value.textContent = \"%d\";" \
"                const brightness_input = document.getElementById(\"brightness-input\");" \
"                brightness_input.value = %d;" \
"                brightness_input.addEventListener(\"input\", (event) => {" \
"                    brightness_value.textContent = event.target.value;" \
"                    fetch(`/ctrls/?brightness=${event.target.value}`, { method: \"POST\" });" \
"                });" \
"            </script>" \
"        </div>" \
"        <div id=\"contrast-ctrl\" style=\"display: flex; align-items: center; gap: 10px\">" \
"            <h3>Contrast:</h3>" \
"            <p id=\"contrast-value\">%d</p>" \
"            <div style=\"display: flex; align-items: center\">" \
"                <strong>0</strong>" \
"                <input id=\"contrast-input\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" />" \
"                <strong>255</strong>" \
"            </div>" \
"            <script>" \
"                const contrast_value = document.getElementById(\"contrast-value\");" \
"                contrast_value.textContent = \"%d\";" \
"                const contrast_input = document.getElementById(\"contrast-input\");" \
"                contrast_input.value = %d;" \
"                contrast_input.addEventListener(\"input\", (event) => {" \
"                    contrast_value.textContent = event.target.value;" \
"                    fetch(`/ctrls/?contrast=${event.target.value}`, { method: \"POST\" });" \
"                });" \
"            </script>" \
"        </div>" \
"        <div id=\"saturation-ctrl\" style=\"display: flex; align-items: center; gap: 10px\">" \
"            <h3>Saturation:</h3>" \
"            <p id=\"saturation-value\">%d</p>" \
"            <div style=\"display: flex; align-items: center\">" \
"                <strong>0</strong>" \
"                <input id=\"saturation-input\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" />" \
"                <strong>255</strong>" \
"            </div>" \
"            <script>" \
"                const saturation_value = document.getElementById(\"saturation-value\");" \
"                saturation_value.textContent = \"%d\";" \
"                const saturation_input = document.getElementById(\"saturation-input\");" \
"                saturation_input.value = %d;" \
"                saturation_input.addEventListener(\"input\", (event) => {" \
"                    saturation_value.textContent = event.target.value;" \
"                    fetch(`/ctrls/?saturation=${event.target.value}`, { method: \"POST\" });" \
"                });" \
"            </script>" \
"        </div>" \
"    </body>" \
"</html>" \
