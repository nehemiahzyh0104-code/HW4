#include "flappy.h"

void advancePipes(Game *game, float dt) {
    for (int i = 0; i < PIPE_COUNT; ++i) {
        game->m_pipes[i].m_x -= PIPE_SPEED * dt;
    }

    /* Recycle only after every pipe has moved: recyclePipe uses current positions. */
    for (int i = 0; i < PIPE_COUNT; ++i) {
        if (game->m_pipes[i].m_x + PIPE_WIDTH <= 0) {
            recyclePipe(game, i);
        }
    }
}

bool birdHitsObstacle(const Game *game) {
    Box bird = gameBirdBox(game);

    if (bird.m_y < 0 || bird.m_y + bird.m_height > FLOOR_Y) {
        return true;
    }

    for (int i = 0; i < PIPE_COUNT; ++i) {
        const Pipe *pipe = &game->m_pipes[i];

        Box upper = {pipe->m_x, 0.0f, PIPE_WIDTH, pipe->m_gap_y};
        Box lower = {pipe->m_x, pipe->m_gap_y + PIPE_GAP_HEIGHT, PIPE_WIDTH,
                     FLOOR_Y - pipe->m_gap_y - PIPE_GAP_HEIGHT};

        if (boxesOverlap(bird, upper) || boxesOverlap(bird, lower)) {
            return true;
        }
    }

    return false;
}

int scorePassedPipes(Game *game) {
    int count = 0;

    for (int i = 0; i < PIPE_COUNT; ++i) {
        /* Strictly left of the bird's left edge; equality is not a pass. */
        if (!game->m_pipes[i].m_scored && game->m_pipes[i].m_x + PIPE_WIDTH < BIRD_X) {
            game->m_pipes[i].m_scored = true;
            ++count;
        }
    }

    game->m_score += count;
    return count;
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
