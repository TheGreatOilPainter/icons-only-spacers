// SPDX-License-Identifier: GPL-2.0-or-later
.pragma library

// Use application identities, never transient row numbers. File/application URLs
// may change when a launcher turns into a running task, but its desktop ID won't.
function keyFor(launcherUrl, appId) {
    let url = String(launcherUrl || "").split(/[?#]/)[0];
    try {
        url = decodeURIComponent(url);
    } catch (error) {
        // A literal percent sign in a desktop filename is valid too.
    }
    if (url.startsWith("preferred://")) {
        return url;
    }
    if (url.startsWith("applications:")) {
        return "desktop:" + url.substring("applications:".length);
    }
    if (url.startsWith("file:") && url.endsWith(".desktop")) {
        return "desktop:" + url.substring(url.lastIndexOf("/") + 1);
    }
    if (url) {
        return "url:" + url;
    }
    return appId ? "app:" + String(appId).replace(/\.desktop$/, "") : "";
}

function contains(anchors, key) {
    return key !== "" && Array.prototype.indexOf.call(anchors, key) !== -1;
}

function updated(anchors, key, enabled) {
    const result = Array.prototype.filter.call(anchors, value => value !== key);
    if (key && enabled) {
        result.push(key);
    }
    return result;
}
