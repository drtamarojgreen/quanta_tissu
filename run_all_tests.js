const fs = require('fs');
const path = require('path');
const { exec } = require('child_process');

const testDirectory = 'tests/model/js';
const reportPath = 'docs/tisslm_npm_test_results.md';

// Find all test files recursively
const testFiles = [];
function findTestFiles(dir) {
    const files = fs.readdirSync(dir);
    for (const file of files) {
        const fullPath = path.join(dir, file);
        if (fs.statSync(fullPath).isDirectory()) {
            findTestFiles(fullPath);
        } else if (file.endsWith('.test.js')) {
            testFiles.push(fullPath);
        }
    }
}
findTestFiles(testDirectory);

const results = [];
let testsCompleted = 0;

console.log(`Found ${testFiles.length} test files. Running...`);

// Execute each test file
testFiles.forEach(file => {
    exec(`node ${file}`, (error, stdout, stderr) => {
        const result = {
            file: file,
            passed: !error,
            output: stdout,
            error: stderr || (error ? error.message : '')
        };
        results.push(result);
        testsCompleted++;

        if (testsCompleted === testFiles.length) {
            generateReport();
        }
    });
});

function generateReport() {
    console.log("All tests finished. Generating report...");

    const passingTests = results.filter(r => r.passed);
    const failingTests = results.filter(r => !r.passed);

    let report = `# TissLM Node.js Model Test Results\n\n`;
    report += `## Automated Test Execution Summary\n\n`;
    report += `An automated execution of the full test suite was performed.\n\n`;
    report += `- **Total Test Suites:** ${results.length}\n`;
    report += `- **Passing:** ${passingTests.length}\n`;
    report += `- **Failing:** ${failingTests.length}\n\n`;
    report += `--- \n\n`;

    if (failingTests.length > 0) {
        report += `## ❌ Failing Tests\n\n`;
        failingTests.forEach(result => {
            report += `### \`${result.file}\`\n\n`;
            report += `**Error:**\n`;
            report += "```\n";
            report += result.error.trim();
            report += "\n```\n\n";
        });
    }

    if (passingTests.length > 0) {
        report += `## ✅ Passing Tests\n\n`;
        report += `The following ${passingTests.length} test suites passed successfully:\n\n`;
        passingTests.forEach(result => {
            report += `- \`${result.file}\`\n`;
        });
    }

    fs.writeFileSync(reportPath, report);
    console.log(`Report generated successfully at ${reportPath}`);
}
