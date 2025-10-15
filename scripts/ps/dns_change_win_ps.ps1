

$adapters = Get-NetAdapter | Where-Object { $_.Status -eq 'Up' }
if(-not $adapters) {
    Write-Error "No active network adapters found."
    exit 1
}

Write-Host "Available network adapters:`n"
$index = 1
foreach ($adapter in $adapters) {
    Write-Host "$index) $($adapter.Name)"
    $index++
}

$selection = Read-Host "Select the adapter number to change DNS settings"
if ($selection -lt 1 -or $selection -gt $adapters.Count) {
    Write-Error "Invalid selection."
    exit 1
}

$chosenAdapter = $adapters[$selection - 1]
Write-Host "`nYou selected: $($chosenAdapter.Name)" -ForegroundColor Cyan

$availableChoices = @(
    "Automatic (DHCP)",
    "Google DNS (8.8.8.8, 8.8.4.4)",
    "Manual (enter your own DNS servers)"
)
$userChoice = Read-Host "Select DNS configuration option number"
foreach ($i in 0..($availableChoices.Count - 1)) {
    Write-Host "$($i + 1)) $($availableChoices[$i])"
}

switch($userChoice) {
    1 {
        Write-Host "Setting DNS to Automatic (DHCP)..." -ForegroundColor Yellow
        Set-DnsClientServerAddress -InterfaceAlias $chosenAdapter.Name -ResetServerAddresses
        Write-Host "DNS set to Automatic (DHCP)." -ForegroundColor Green
    }
    2 {
        Write-Host "Setting DNS to Google DNS (8.8.8.8, 8.8.4.4)..." -ForegroundColor Yellow
        Set-DnsClientServerAddress -InterfaceAlias $chosenAdapter.Name -ServerAddresses ("8.8.8.8", "8.8.4.4")
        Write-Host "DNS set to Google DNS (8.8.8.8, 8.8.4.4)." -ForegroundColor Green
    }
    3 {
        $manualDns = Read-Host "Enter the DNS server addresses (comma-separated)"
        $dnsArray = $manualDns -split ","
        Write-Host "Setting DNS to Manual ($manualDns)..." -ForegroundColor Yellow
        Set-DnsClientServerAddress -InterfaceAlias $chosenAdapter.Name -ServerAddresses $dnsArray
        Write-Host "DNS set to Manual ($manualDns)." -ForegroundColor Green
    }
    Default {
        Write-Error "Invalid selection."
        exit 1
    }

Write-Host "`nCurrent DNS settings for $($chosenAdapter.Name):" -ForegroundColor Cyan
Get-DnsClientServerAddress -InterfaceAlias $chosenAdapter.Name | ForEach-Object {
    Write-Host " - $($_.ServerAddresses -join ', ')" -ForegroundColor Green
}
pause