import os
import sys
import json

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.db.lite_client import TissDBLiteClient

def run_tissdb_lite_tests():
    report = ["\n--- Test 8: TissDB-Lite (JavaScript) Integration ---"]
    client = TissDBLiteClient()

    try:
        # Ensure DB setup (clears in-memory DB)
        client.delete_db()
        client.ensure_db_setup(['myCollection'])
        report.append("  TissDBLite setup and collection created.")

        # Insert items
        item1 = client.add_document('myCollection', { 'name': 'Test Item 1', 'value': 10 })
        item2 = client.add_document('myCollection', { 'name': 'Test Item 2', 'value': 20 })
        report.append(f"  Inserted item 1: {item1['name']}")
        report.append(f"  Inserted item 2: {item2['name']}")

        # Find items
        found_items = client.find('myCollection', 'value > 15')
        report.append(f"  Found items (value > 15): {len(found_items)} item(s).")
        if found_items:
            report.append(f"    Found: {found_items[0]['name']}")

        # Update item
        item1['value'] = 15
        updated_item = client.update('myCollection', item1)
        report.append(f"  Updated item 1: {updated_item['name']} to value {updated_item['value']}")

        # Remove item
        client.remove('myCollection', item2['_id'])
        report.append(f"  Removed item 2: {item2['_id']}")

        # Export collection to verify
        all_items = client.export_collection('myCollection')
        report.append(f"  All items after removal: {len(all_items)} item(s).")
        if all_items:
            report.append(f"    Remaining: {all_items[0]['name']}")

        report.append("  TissDB-Lite tests completed successfully.")

    except Exception as e:
        report.append(f"  [ERROR] TissDB-Lite test failed: {e}")
    finally:
        # Clean up (clear in-memory DB)
        client.delete_db()
        report.append("  TissDBLite cleanup complete.")

    return report

if __name__ == '__main__':
    report = run_tissdb_lite_tests()
    for line in report:
        print(line)
