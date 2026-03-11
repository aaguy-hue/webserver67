document.getElementById('statusBtn').addEventListener('click', function() {
    const statusMsg = document.getElementById('statusMsg');
    statusMsg.textContent = '✓ Server is online and responding!';
    statusMsg.style.color = '#28a745';
});

alert('Webserver67 loaded successfully');
