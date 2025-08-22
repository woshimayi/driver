/*
 * @*************************************:
 * @FilePath     : /user/C/string/qos_weight.c
 * @version      :
 * @Author       : dof
 * @Date         : 2025-07-07 14:12:48
 * @LastEditors  : dof
 * @LastEditTime : 2025-07-10 13:42:09
 * @Descripttion :
 * @compile      :
 * @**************************************:
 */

int check_weight(int *current_wei, int *current)
{
    int default_weight[8] = {20, 20, 15, 15, 10, 10, 5, 5};

    // int current_wei[8] = {40, 20, 10, 10, 0, 10, 0, 10};
    // int current_wei[8] = {20, 0, 0, 0, 0, 0, 0, 0};
    int current_old[8] = {0};

    int total_zero = 8;

    int weight[8] = {1, 1, 1, 1, 1, 1, 1, 1};
    int idx[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    int totalWeight = 0;

    for (int i = 0; i < 8; i++)
    {
        /*save weight values*/
        if (current_wei[i] > 0)
        {
            current_old[i] = current_wei[i];

            totalWeight += current_wei[i];
            weight[i] = current_wei[i];
            total_zero--;
        }
        else
        {
            current_old[i] = 1;
        }
    }

    if (0 == totalWeight || 8 > totalWeight)
    {
        memcpy(current_old, default_weight, sizeof(default_weight));
    }
    else
    {
        int wei = 0;
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8 - 1 - i; j++)
            {
                if (weight[j] < weight[j + 1])
                {
                    wei = weight[j];
                    weight[j] = weight[j + 1];
                    weight[j + 1] = wei;

                    wei = idx[j];
                    idx[j] = idx[j + 1];
                    idx[j + 1] = wei;
                }
            }
        }

        for (int n = 0; n < total_zero; n++)
        {
            current_old[idx[n % (8 - total_zero)]] -= 1;
        }
    }

    memcpy(current, current_old, sizeof(current_old));

    return 0;
}

int main(int argc, char const *argv[])
{
    int current_old[8] = {0};
    int current_wei[8] = {40, 20, 10, 10, 0, 10, 0, 10};

    check_weight(current_wei, &current_old[0]);
    return 0;
}

