/**
 * [KalmanFilter 卡尔曼滤波算法]
 * @param ResrcDataCnt      [description]
 * @param ResrcData         [description]
 * @param FilterOutput      [description]
 * @param ProcessNiose_Q    [description]
 * @param MeasureNoise_R    [description]
 * @param InitialPrediction [description]
 */
void KalmanFilter(unsigned int ResrcDataCnt,const double *ResrcData,double *FilterOutput,double ProcessNiose_Q,double MeasureNoise_R,double InitialPrediction)
{
        unsigned int i;
        double R = MeasureNoise_R;
        double Q = ProcessNiose_Q;
        double x_last = *ResrcData;
        double x_mid = x_last;
        double x_now;
        double p_last = InitialPrediction;
        double p_mid ;
        double p_now;
        double kg;

        for(i=0;i<ResrcDataCnt;i++)
        {
                x_mid=x_last; //x_last=x(k-1|k-1),x_mid=x(k|k-1)
                p_mid=p_last+Q; //p_mid=p(k|k-1),p_last=p(k-1|k-1),Q=噪声
                kg=p_mid/(p_mid+R); //kg为kalman filter，R为噪声
                x_now=x_mid+kg*(*(ResrcData+i)-x_mid);//估计出的最优值
                p_now=(1-kg)*p_mid;//最优值对应的covariance
                
                *(FilterOutput + i)  = x_now;

                p_last = p_now; //更新covariance值
                x_last = x_now; //更新系统状态值
        }

}