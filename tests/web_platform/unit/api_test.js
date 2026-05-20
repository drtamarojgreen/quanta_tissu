const assert = require('node:assert');
const test = require('node:test');
const PlatformAPI = require('../../../web_platform/frontend/js/api/platform_api.js');

test('PlatformAPI.switch_tab should update state', async (t) => {
    const result = await PlatformAPI.switch_tab('playground');
    assert.strictEqual(result.status, 'success');
    assert.strictEqual(result.active_tab, 'playground');
});

test('PlatformAPI.get_state should return AppState', async (t) => {
    const state = await PlatformAPI.get_state();
    assert.ok(state.tabs);
    assert.ok(state.tasks);
});

test('PlatformAPI.list_tasks should return tasks object', async (t) => {
    const tasks = await PlatformAPI.list_tasks();
    assert.strictEqual(typeof tasks, 'object');
});
