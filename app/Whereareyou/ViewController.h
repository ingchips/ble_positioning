//
//  ViewController.h
//  Whereareyou
//
//  Created by yfan on 2022/4/20.
//

#import <UIKit/UIKit.h>

@protocol PassValue <NSObject>

- (void)PassValue:(NSDictionary *)localDic;

@end


@interface ViewController : UIViewController
@property (nonatomic,weak) id delegate;

@end

