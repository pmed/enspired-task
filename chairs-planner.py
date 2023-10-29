import sys
import re
import fileinput
import unittest
from collections import deque

CHAIR_TYPES = ['W', 'P', 'S', 'C']
WALL_TYPES = ['+', '-', '|', '\\', '/', '\n']
VISITED = 'X'

class Room:
    def __init__(self, name: str, x: int = 0, y: int = 0):
        """
        Create a room with name, and optional x and y coordinates.
        """
        self.name = name
        self.x = x
        self.y = y
        self.chairs = {type: 0 for type in CHAIR_TYPES}

    def chairs_str(self):
        return ', '.join([f'{type}: {count}' for type, count in self.chairs.items()])


class Plan:
    def __init__(self):
        self.plan = []

    def _get(self, x, y):
        if 0 <= y < len(self.plan) and 0 <= x < len(self.plan[y]):
            return self.plan[y][x]
        else:
            return None

    def _set(self, x, y, value):
        if 0 <= y < len(self.plan) and 0 <= x < len(self.plan[y]):
            self.plan[y] = self.plan[y][:x] + value + self.plan[y][x + 1:]

    # read plan as bitmap from a file or stdin (when filename is None), check bounds and shape
    def read(self, filename):
        self.plan = []
        for line in fileinput.input(filename):
            self.plan.append(line)

    def find_chairs_in_rooms(self) -> list[Room]:
        total = Room('total')
        rooms = self._find_rooms()
        for room in rooms:
            self._find_chairs(room, total)
        return [total, *rooms]

    def _find_rooms(self) -> list[Room]:
        '''
        Returns rooms list sorted by name
        Room names will be erased on self.plan
        '''
        found = dict()
        pattern = r'\(([^)]*)\)'
        for row, line in enumerate(self.plan):
            replaced = ''
            for match in re.finditer(pattern, line):
                name = match.group(1).strip()
                start = match.start()
                end = match.end()
                pos = (row, start)
                if name == '':
                    raise RuntimeError(f'Empty room name at {pos}')
                if name in found:
                    raise RuntimeError(f'Duplicate room name {name}, initially defined at {found[name]}')
                found[name] = pos
                replaced = replaced[:start] if replaced else line[:start]
                replaced += ' ' * (end - start)
                replaced += line[end:]
            if replaced:
                self.plan[row] = replaced

        return [Room(name, x, y) for name, (y, x) in sorted(found.items())]


    def _find_chairs(self, room: Room, total: Room):
        '''
        Use non-recursive flood fill algorithm with 4 directions
        (see https://en.wikipedia.org/wiki/Flood_fill)
        Visited cells will be marked as X on the plan
        '''
        directions = [(0, 1), (0, -1), (1, 0), (-1, 0)]
    
        q = deque([(room.x, room.y)])
        while q:
            x, y = q.popleft()
            cell = self._get(x, y)
            if cell == VISITED:
                continue
            if cell in CHAIR_TYPES:
                # found a chair
                room.chairs[cell] += 1
                total.chairs[cell] += 1
            # mark visited
            self._set(x, y, VISITED)
            # explore all directions (BFS)
            for dx, dy in directions:
                new_x = x + dx
                new_y = y + dy
                cell = self._get(new_x, new_y)
                if cell and (cell != VISITED) and (cell not in WALL_TYPES):
                    q.append((new_x, new_y))

class RoomTests(unittest.TestCase):
    def test_init(self):
        room1 = Room('room1')
        self.assertEqual(room1.name, 'room1')
        self.assertEqual(room1.x, 0)
        self.assertEqual(room1.y, 0)
        self.assertEqual(room1.chairs, dict(W=0, P=0, S=0, C=0))

    def test_chairs_str(self):
        room2 = Room('room 2', 1, 2)
        self.assertEqual(room2.name, 'room 2')
        self.assertEqual(room2.x, 1)
        self.assertEqual(room2.y, 2)
        self.assertEqual(room2.chairs, dict(W=0, P=0, S=0, C=0))
        self.assertEqual(room2.chairs_str(), "W: 0, P: 0, S: 0, C: 0")

        room2.chairs['P'] += 3
        room2.chairs['C'] = 4
        self.assertEqual(room2.chairs, dict(W=0, P=3, S=0, C=4))
        self.assertEqual(room2.chairs_str(), "W: 0, P: 3, S: 0, C: 4")


class PlanTests(unittest.TestCase):
    def test_init(self):
        plan = Plan()
        self.assertEqual(plan.plan, [])

    # Returns the value at the given x, y coordinates when they are within the bounds of the plan
    def test_cell_access(self):
        plan = Plan()
        plan.plan = ['abc123',
                     'def',
                     'ghijklmn']
        self.assertEqual(plan._get(0, 0), 'a')
        self.assertEqual(plan._get(1, 1), 'e')
        self.assertEqual(plan._get(6, 2), 'm')
        self.assertEqual(plan._get(-1, -99), None)
        self.assertEqual(plan._get(4, 1), None)

        plan._set(1, 2, 'X')
        plan._set(4, 0, 'Y')
        plan._set(-1, -1, 'Q')
        plan._set(10, 0, 'W')
        self.assertEqual(plan.plan, ['abc1Y3',
                                     'def',
                                     'gXijklmn'])

    def test_find_rooms(self):
        plan = Plan()
        plan.plan = ['+-----------------+',
                     '|(A) ( long name )|',
                     '+-----------------+']
    
        rooms = plan._find_rooms()
        self.assertEqual(len(rooms), 2)
        self.assertEqual(rooms[0].name, 'A')
        self.assertEqual(rooms[0].x, 1)
        self.assertEqual(rooms[0].y, 1)
        self.assertEqual(rooms[1].name, 'long name')
        self.assertEqual(rooms[1].x, 5)
        self.assertEqual(rooms[1].y, 1)
        self.assertEqual(plan.plan, ['+-----------------+',
                                     '|                 |',
                                     '+-----------------+'])
        # second call finds nothing more
        rooms = plan._find_rooms()
        self.assertEqual(len(rooms), 0)

    def test_find_no_rooms(self):
        plan = Plan()
        plan.plan = ['+----+',
                     '|    |',
                     '+----+']
        rooms = plan._find_rooms()
        self.assertEqual(rooms, [])
        self.assertEqual(plan.plan, ['+----+',
                                     '|    |',
                                     '+----+'])

    def test_empty_room_name(self):
        plan = Plan()
        plan.plan = ['+-+',
                     '|()|',
                     '+-+']
        with self.assertRaises(RuntimeError):
            plan._find_rooms()

    def test_duplicate_room_name(self):
        plan = Plan()
        plan.plan = ['+-+',
                     '|(A)|',
                     '|(A)|',
                     '+-+']
        with self.assertRaises(RuntimeError):
            plan._find_rooms()

    def test_find_chairs(self):
        plan = Plan()
        plan.plan = """
+-----------+------------------------------------+
|           |                                    |
| (closet)  |                                    |
|         P |                            S       |
|         P |         (sleeping room)            |
|         P |                                    |
|           |                                    |
+-----------+    W                               |
|           |                                    |
|        W  |                                    |
|           |                                    |
|           +--------------+---------------------+
|                          |                     |
|                          |                W W  |
|                          |    (office)         |
|                          |                     |
+--------------+           |                     |
|              |           |                     |
| (toilet)     |           |             P       |
|   C          |           |                     |
|              |           |                     |
+--------------+           +---------------------+
|              |           |                     |
|              |           |                     |
|              |           |                     |
| (bathroom)   |           |      (kitchen)      |
|              |           |                     |
|              |           |      W   W          |
|              |           |      W   W          |
|       P      +           |                     |
|             /            +---------------------+
|            /                                   |
|           /                                    |
|          /                          W    W   W |
+---------+                                      |
|                                                |
| S                                   W    W   W |
|                (living room)                   |
| S                                              |
|                                                |
|                                                |
|                                                |
|                                                |
+--------------------------+---------------------+
                           |                     |
                           |                  P  |
                           |  (balcony)          |
                           |                 P   |
                           |                     |
                           +---------------------+
""".splitlines()
        # find chairs
        found = {}
        for room in plan.find_chairs_in_rooms():
            found[room.name] = room.chairs
        self.assertEqual(found, {
            'total': {'W': 14, 'P': 7, 'S': 3, 'C': 1 },
            'balcony': { 'W': 0, 'P': 2, 'S': 0, 'C': 0 },
            'bathroom': { 'W': 0, 'P': 1, 'S': 0, 'C': 0 },
            'closet': { 'W': 0, 'P': 3, 'S': 0, 'C': 0 },
            'kitchen': { 'W': 4, 'P': 0, 'S': 0, 'C': 0 },
            'living room': { 'W': 7, 'P': 0, 'S': 2, 'C': 0 },
            'office': { 'W': 2, 'P': 1, 'S': 0, 'C': 0 },
            'sleeping room': { 'W': 1, 'P': 0, 'S': 1, 'C': 0 },
            'toilet': { 'W': 0, 'P': 0, 'S': 0, 'C': 1 },
        })

        # all cells are visited
        for row in plan.plan:
            for cell in row.strip():
                self.assertTrue(cell == VISITED or cell in WALL_TYPES)

def main():
    filename = sys.argv[1:2] # argv[1] or None

    plan = Plan()
    plan.read(filename)

    rooms = plan.find_chairs_in_rooms()

    # output in specified format
    for room in rooms:
        print(f'{room.name}:\n{room.chairs_str()}')

if __name__ == '__main__':
    main()
