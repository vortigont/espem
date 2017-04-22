/*
    This script intercepts form send action, retrieves data and sends in as a JSON object.
    Also it retrieves form data via AJAX request and fills the form.
    No jQuery required.
    Both GET/POST supported.
    Multiple values checkboxes supported.
    MultiSelect fileds filling not tested.

    Based on tutor  https://code.lengstorf.com/get-form-values-as-json/

	Should be minimized before uploading to the esp (for e.x. via https://jscompress.com/)
*/

var formurl = "cfg";
var formid = "fcfg";

const handleFormSubmit = event => {
  event.preventDefault();
  const data = formToJSON(form.elements);
  ajax_request(formurl, 'POST', data, function(data){process_resp(data);});
  alert("Saving config. Autoreboot in 20 sec...");
};

const formToJSON = elements => [].reduce.call(elements, (data, element) => {
  if (isValidElement(element) && isValidValue(element)) {
    if (isCheckbox(element)) {
      data[element.name] = (data[element.name] || []).concat(element.value);
    } else if (isMultiSelect(element)) {
      data[element.name] = getSelectValues(element);
    } else {
      data[element.name] = element.value;
    }
  }
  return data;
}, {});


const isValidElement = element => {
  return element.name && element.value;
};
const isValidValue = element => {
  return (!['checkbox', 'radio'].includes(element.type) || element.checked);
};
const isCheckbox = element => element.type === 'checkbox';
const isMultiSelect = element => element.options && element.multiple;
const getSelectValues = options => [].reduce.call(options, (values, option) => {
  return option.selected ? values.concat(option.value) : values;
}, []);

//AJAX req function
const ajax_request = (url, method, data, callback) => {
    xhr = new XMLHttpRequest();
    if (method == "GET") url = url + "?data=" + encodeURIComponent(JSON.stringify(data));
    xhr.open(method, url, true);
    xhr.setRequestHeader("Content-type", "application/json");
    xhr.onreadystatechange = function () {
        if (xhr.readyState == 4 && xhr.status == 200) {
            callback(JSON.parse(xhr.responseText));
        }
    }

    if (method == "GET") {
        xhr.send();
    } else {
        xhr.send(JSON.stringify(data));
    }
}

const tickbox = (form_obj, jdata_obj) => {
    if (isCheckbox(form_obj) && form_obj.value === jdata_obj) {
        form_obj.checked = true; return true;
    }
    return false;        
}

const process_resp = (jdata) => {
    var frm = document.getElementById(formid);
    frm.reset();
    for (var i in jdata) {
        if( Object.prototype.toString.call( jdata[i] ) === '[object Array]' ) {
            for (var j in jdata[i]){
                if (frm.elements[i].length) {
                    for (var k in frm.elements[i]) { tickbox(frm.elements[i][k], jdata[i][j]); }
                } else { tickbox(frm.elements[i], jdata[i][j]); }
                }
              } else {
                frm.elements[i].value = jdata[i];
        }
    }

};

const form = document.getElementById(formid);
form.addEventListener('submit', handleFormSubmit);

window.onload = function() {
    ajax_request(formurl, 'GET', "", function(data){process_resp(data);});
}