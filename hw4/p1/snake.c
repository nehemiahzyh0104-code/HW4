#include "snake.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

static void freeRing(SnakeNode *head) {
    if (head == NULL) {
        return;
    }

    SnakeNode *node = head->m_next;
    while (node != head) {
        SnakeNode *next = node->m_next;
        free(node);
        node = next;
    }
    free(head);
}

static bool isOppositeDirection(Direction first, Direction second) {
    return (first == DIR_UP && second == DIR_DOWN) || (first == DIR_DOWN && second == DIR_UP) ||
           (first == DIR_LEFT && second == DIR_RIGHT) || (first == DIR_RIGHT && second == DIR_LEFT);
}

static Direction effectiveDirection(const Snake *snake, Direction requested) {
    if ((int)requested < 0 || requested >= DIRECTION_COUNT) {
        return snake->m_direction;
    }

    if (snake->m_length > 1 && isOppositeDirection(requested, snake->m_direction)) {
        return snake->m_direction;
    }

    return requested;
}

static Coord adjacentCoord(Coord pos, Direction direction) {
    switch (direction) {
    case DIR_UP:
        --pos.m_row;
        break;
    case DIR_RIGHT:
        ++pos.m_col;
        break;
    case DIR_DOWN:
        ++pos.m_row;
        break;
    case DIR_LEFT:
        --pos.m_col;
        break;
    default:
        break;
    }
    return pos;
}

bool snakeInit(Snake *snake, int rows, int cols, size_t length) {
    if (rows < MIN_BOARD_SIZE || cols < MIN_BOARD_SIZE || length == 0 ||
        length > (size_t)(cols - 2 * BOARD_WALL_THICKNESS)) {
        *snake = (Snake){0};
        return false;
    }

    int headRow = rows / 2;
    int headCol = cols / 2;
    if (length > (size_t)headCol) {
        headCol = (int)length;
    }

    SnakeNode *head = malloc(sizeof(*head));
    if (head == NULL) {
        *snake = (Snake){0};
        return false;
    }

    head->m_pos = (Coord){headRow, headCol};
    head->m_next = head;

    /* Insert every new node after the head, so head->m_next stays the tail. */
    for (size_t i = 1; i < length; ++i) {
        SnakeNode *node = malloc(sizeof(*node));
        if (node == NULL) {
            freeRing(head);
            *snake = (Snake){0};
            return false;
        }
        node->m_pos = (Coord){headRow, headCol - (int)i};
        node->m_next = head->m_next;
        head->m_next = node;
    }

    snake->m_head = head;
    snake->m_length = length;
    snake->m_direction = DIR_RIGHT;
    snake->m_rows = rows;
    snake->m_cols = cols;
    return true;
}

Coord snakeNextPosition(const Snake *snake, Direction requested) {
    Direction direction = effectiveDirection(snake, requested);
    return adjacentCoord(snake->m_head->m_pos, direction);
}

bool snakeContains(const Snake *snake, Coord pos) {
    const SnakeNode *node = snake->m_head;
    if (node == NULL || snake->m_length == 0) {
        return false;
    }

    for (size_t i = 0; i < snake->m_length; ++i) {
        if (node->m_pos.m_row == pos.m_row && node->m_pos.m_col == pos.m_col) {
            return true;
        }
        node = node->m_next;
    }
    return false;
}

StepResult snakeStep(Snake *snake, Direction requested, bool grow) {
    Direction direction = effectiveDirection(snake, requested);
    Coord next = adjacentCoord(snake->m_head->m_pos, direction);

    bool hitsWall =
        next.m_row < BOARD_WALL_THICKNESS || next.m_row >= snake->m_rows - BOARD_WALL_THICKNESS ||
        next.m_col < BOARD_WALL_THICKNESS || next.m_col >= snake->m_cols - BOARD_WALL_THICKNESS;
    if (hitsWall) {
        return STEP_WALL;
    }

    if (!grow) {
        /* The old tail vacates its cell, so it is excluded from the body check. */
        SnakeNode *oldTail = snake->m_head->m_next;
        const SnakeNode *node = oldTail->m_next;
        for (size_t i = 0; i + 1 < snake->m_length; ++i) {
            if (node->m_pos.m_row == next.m_row && node->m_pos.m_col == next.m_col) {
                return STEP_SELF;
            }
            node = node->m_next;
        }

        oldTail->m_pos = next;
        snake->m_head = oldTail;
        snake->m_direction = direction;
        return STEP_OK;
    }

    /* Growth keeps every node, so the old tail cell remains occupied. */
    const SnakeNode *node = snake->m_head;
    for (size_t i = 0; i < snake->m_length; ++i) {
        if (node->m_pos.m_row == next.m_row && node->m_pos.m_col == next.m_col) {
            return STEP_SELF;
        }
        node = node->m_next;
    }

    SnakeNode *newHead = malloc(sizeof(*newHead));
    if (newHead == NULL) {
        return STEP_NO_MEMORY;
    }

    newHead->m_pos = next;
    newHead->m_next = snake->m_head->m_next;
    snake->m_head->m_next = newHead;
    snake->m_head = newHead;
    ++snake->m_length;
    snake->m_direction = direction;
    return STEP_OK;
}

void snakeDestroy(Snake *snake) {
    freeRing(snake->m_head);
    *snake = (Snake){0};
}

unsigned int nextRandom(unsigned int seed) {
    static unsigned int state = RANDOM_INITIAL_SEED;

    /* A positive seed resets before the recurrence step; zero continues. */
    if (seed > 0) {
        state = seed % RANDOM_MODULUS;
    }

    state = (RANDOM_MULTIPLIER * state + RANDOM_INCREMENT) % RANDOM_MODULUS;
    return state;
}
