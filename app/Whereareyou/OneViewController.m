//
//  OneViewController.m
//  Whereareyou
//
//  Created by yfan on 2022/4/21.
//

#import "OneViewController.h"
#define width [UIScreen mainScreen].bounds.size.width
#define height [UIScreen mainScreen].bounds.size.height


@interface OneViewController (){
    UIImageView *local;
    float  localX;
    float  localY;
}
@property (weak, nonatomic) IBOutlet UILabel *tagLab;

@end

@implementation OneViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = @"二维平面定位";
    UIView *xView = [[UIView alloc] initWithFrame:CGRectMake(0, height/2, width, 1)];
    xView.backgroundColor = UIColor.lightGrayColor;
    [self.view addSubview:xView];
    
    UIView *yView = [[UIView alloc] initWithFrame:CGRectMake(width/2, 0, 1, height)];
    yView.backgroundColor = UIColor.lightGrayColor;
    [self.view addSubview:yView];
    
    //icon
    local = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"local"]];
    local.frame = CGRectMake(width/2-10, height/2-10, 20, 20);
    [self.view addSubview:local];
    
    //(0,0)坐标
}
- (void)PassValue:(NSDictionary *)localDic{
    NSArray *tagArr = [localDic objectForKey:@"id"];
    
    self.tagLab.text = [NSString stringWithFormat:@"%@ \nx:%@ \ny:%@ \nz:%@",tagArr[0],[localDic objectForKey:@"x"],[localDic objectForKey:@"y"],[localDic objectForKey:@"z"]];
    
    float x = [[localDic objectForKey:@"x"] floatValue];
    float y = [[localDic objectForKey:@"y"] floatValue];
    //扩大20倍显示
    localX = width/2-10 + x*60;
    localY = height/2-10 - y*60;
//    local.frame = CGRectMake(localX, localY, 20, 20);
    
    //动画
    CABasicAnimation *moveAnimate=[CABasicAnimation animationWithKeyPath:@"position"];
    //设置动画初始位置
    moveAnimate.fromValue=[NSValue valueWithCGPoint:CGPointMake(localX, localY)];
    //目的位置
    moveAnimate.toValue= [NSValue valueWithCGPoint:CGPointMake(localX, localY)];
    moveAnimate.duration = 1;
    //设置动画速率
    moveAnimate.timingFunction=[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn];

    moveAnimate.removedOnCompletion=NO;
    moveAnimate.fillMode=kCAFillModeForwards;
    [local.layer addAnimation:moveAnimate forKey:@"animation"];
    
    
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
