var SPREADSHEET_ID = '1f0Dk-0KQysMJmqzEjupM-9yyNKDlv95r56B2guHWn08';
var SHEET_NAME = 'Sheet1';
var CONFIG_SHEET_NAME = 'Config';
var MESSAGE_COST_BULK = 5000;
var MESSAGE_COST_SINGLE = 10000;

function getOrCreateConfigSheet(ss) {
  var configSheet = ss.getSheetByName(CONFIG_SHEET_NAME);
  if (!configSheet) {
    configSheet = ss.insertSheet(CONFIG_SHEET_NAME);
    configSheet.appendRow(['Key', 'Value']);
    configSheet.appendRow(['PhoneNumber', '']);
    configSheet.appendRow(['SerialPort', '']);
    configSheet.appendRow(['TempThreshold', '35.0']);
  }
  return configSheet;
}

function getConfigValue(ss, key) {
  var configSheet = getOrCreateConfigSheet(ss);
  var data = configSheet.getDataRange().getValues();
  for (var i = 1; i < data.length; i++) {
    if (data[i][0].toString() == key) {
      return data[i][1].toString();
    }
  }
  return '';
}

function setConfigValue(ss, key, value) {
  var configSheet = getOrCreateConfigSheet(ss);
  var data = configSheet.getDataRange().getValues();
  for (var i = 1; i < data.length; i++) {
    if (data[i][0].toString() == key) {
      configSheet.getRange(i + 1, 2).setValue(value);
      return true;
    }
  }
  configSheet.appendRow([key, value]);
  return true;
}

function doGet(e) {
  var action = e.parameter.action;
  var mac = e.parameter.mac;
  var name = e.parameter.name;
  var messages = parseInt(e.parameter.messages);
  var key = e.parameter.key;
  var value = e.parameter.value;
  
  var ss = SpreadsheetApp.openById(SPREADSHEET_ID);
  var sheet = ss.getSheetByName(SHEET_NAME) || ss.getSheets()[0];
  
  var result = {success: false, message: ''};
  
  try {
    switch (action) {
      case 'register':
        result = registerUser(sheet, name, mac);
        break;
      case 'get':
        result = getUser(sheet, mac);
        break;
      case 'deduct':
        result = deductBalance(sheet, mac);
        break;
      case 'topup':
        result = topUpBalance(sheet, mac, messages);
        break;
      case 'check':
        result = checkUser(sheet, mac);
        break;
      case 'getconfig':
        result = {success: true, key: key, value: getConfigValue(ss, key)};
        break;
      case 'setconfig':
        setConfigValue(ss, key, value);
        result = {success: true, message: 'Config updated', key: key, value: value};
        break;
      default:
        result = {success: false, message: 'Unknown action'};
    }
  } catch (err) {
    result = {success: false, message: err.toString()};
  }
  
  return ContentService.createTextOutput(JSON.stringify(result))
    .setMimeType(ContentService.MimeType.JSON);
}

function registerUser(sheet, name, mac) {
  var data = sheet.getDataRange().getValues();
  
  for (var i = 1; i < data.length; i++) {
    if (data[i][1].toString().toLowerCase() == mac.toString().toLowerCase()) {
      return {success: false, message: 'User already exists', name: data[i][0], mac: data[i][1], balance: data[i][2], state: data[i][3]};
    }
  }
  
  sheet.appendRow([name, mac.toUpperCase(), 10000, 'ng']);
  
  return {success: true, message: 'User registered', name: name, mac: mac.toUpperCase(), balance: 10000, state: 'ng'};
}

function getUser(sheet, mac) {
  var data = sheet.getDataRange().getValues();
  
  for (var i = 1; i < data.length; i++) {
    if (data[i][1].toString().toLowerCase() == mac.toString().toLowerCase()) {
      return {success: true, message: 'User found', name: data[i][0], mac: data[i][1], balance: data[i][2], state: data[i][3]};
    }
  }
  
  return {success: false, message: 'User not found'};
}

function checkUser(sheet, mac) {
  var data = sheet.getDataRange().getValues();
  
  for (var i = 1; i < data.length; i++) {
    if (data[i][1].toString().toLowerCase() == mac.toString().toLowerCase()) {
      return {success: true, message: 'User exists', name: data[i][0], mac: data[i][1], balance: data[i][2], state: data[i][3]};
    }
  }
  
  return {success: false, message: 'User not found'};
}

function deductBalance(sheet, mac) {
  var data = sheet.getDataRange().getValues();
  
  for (var i = 1; i < data.length; i++) {
    if (data[i][1].toString().toLowerCase() == mac.toString().toLowerCase()) {
      var balance = data[i][2];
      var currentMessages = Math.floor(balance / MESSAGE_COST_BULK);
      var costPerMessage = (currentMessages >= 10) ? MESSAGE_COST_BULK : MESSAGE_COST_SINGLE;
      var cost = costPerMessage;
      
      if (balance < cost) {
        return {success: false, message: 'Insufficient balance', balance: balance, cost: cost};
      }
      
      var newBalance = balance - cost;
      sheet.getRange(i + 1, 3).setValue(newBalance);
      
      return {success: true, message: 'Balance deducted', oldBalance: balance, newBalance: newBalance, cost: cost};
    }
  }
  
  return {success: false, message: 'User not found'};
}

function topUpBalance(sheet, mac, messageCount) {
  var data = sheet.getDataRange().getValues();
  
  if (!messageCount || messageCount <= 0) {
    return {success: false, message: 'Invalid message count'};
  }
  
  var costPerMessage = (messageCount >= 10) ? MESSAGE_COST_BULK : MESSAGE_COST_SINGLE;
  var totalCost = messageCount * costPerMessage;
  
  for (var i = 1; i < data.length; i++) {
    if (data[i][1].toString().toLowerCase() == mac.toString().toLowerCase()) {
      var balance = data[i][2];
      var newBalance = balance + totalCost;
      sheet.getRange(i + 1, 3).setValue(newBalance);
      
      return {
        success: true,
        message: 'Balance updated',
        oldBalance: balance,
        newBalance: newBalance,
        messagesAdded: messageCount,
        costPerMessage: costPerMessage,
        totalCost: totalCost
      };
    }
  }
  
  return {success: false, message: 'User not found'};
}