'use client'

import { useMemo } from 'react'

interface ArbitrageOpportunity {
  event_id: string
  buy_market: number
  sell_market: number
  buy_price: number
  sell_price: number
  profit_percentage: number
  max_size: number
}

interface OpportunitiesListProps {
  opportunities: ArbitrageOpportunity[]
}

const marketNames: { [key: number]: string } = {
  0: 'Polymarket',
  1: 'Kalshi',
  2: 'PredictIt'
}

export default function OpportunitiesList({ opportunities }: OpportunitiesListProps) {
  // Sort opportunities by profit percentage (highest first)
  const sortedOpportunities = useMemo(() => {
    return [...opportunities].sort((a, b) => b.profit_percentage - a.profit_percentage)
  }, [opportunities])

  return (
    <div className="card">
      <div className="card-header">
        <h2>Arbitrage Opportunities</h2>
        {sortedOpportunities.length > 0 && (
          <span style={{ fontSize: '0.8125rem', color: '#888888' }}>
            {sortedOpportunities.length} active
          </span>
        )}
      </div>
      <div className="card-content">
        {sortedOpportunities.length === 0 ? (
          <div className="empty-state">
            No opportunities detected yet. Waiting for market data...
          </div>
        ) : (
          <div className="opportunities-list">
            {sortedOpportunities.map((opp, index) => {
              const profitPercent = opp.profit_percentage * 100
              const isHighProfit = profitPercent > 1.0
              
              return (
                <div key={index} className={`opportunity-card ${isHighProfit ? 'high-profit' : ''}`}>
                  <div className="opportunity-header">
                    <div className="event-name">{opp.event_id}</div>
                    {isHighProfit && (
                      <div className="profit-badge">
                        +{profitPercent.toFixed(2)}%
                      </div>
                    )}
                  </div>
                  
                  <div className="opportunity-details">
                    <div className="detail-item">
                      <div className="detail-label">Buy Market</div>
                      <div className="detail-value">
                        <span className="market-tag">{marketNames[opp.buy_market] || `Market ${opp.buy_market}`}</span>
                        <span>{(opp.buy_price * 100).toFixed(2)}¢</span>
                      </div>
                    </div>
                    
                    <div className="detail-item">
                      <div className="detail-label">Sell Market</div>
                      <div className="detail-value">
                        <span className="market-tag">{marketNames[opp.sell_market] || `Market ${opp.sell_market}`}</span>
                        <span>{(opp.sell_price * 100).toFixed(2)}¢</span>
                      </div>
                    </div>
                  </div>
                  
                  <div className="opportunity-footer">
                    <span>Max Size: ${opp.max_size.toFixed(2)}</span>
                    <span style={{ color: isHighProfit ? '#10b981' : '#888888', fontWeight: 600 }}>
                      {profitPercent.toFixed(2)}% profit
                    </span>
                  </div>
                </div>
              )
            })}
          </div>
        )}
      </div>
    </div>
  )
}
